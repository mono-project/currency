#!/usr/bin/env python3

import argparse
import datetime
import math
import random
import statistics
import sys
import time
from collections import namedtuple
from functools import partial
from operator import attrgetter

def bits_to_target(bits):
    size = bits >> 24
    assert size <= 0x1d

    word = bits & 0x00ffffff
    assert 0x8000 <= word <= 0x7fffff

    if size <= 3:
        return word >> (8 * (3 - size))
    else:
        return word << (8 * (size - 3))

MAX_BITS = 0x1d00ffff
MAX_TARGET = bits_to_target(MAX_BITS)

def target_to_bits(target):
    assert target > 0
    if target > MAX_TARGET:
        print('Warning: target went above maximum ({} > {})'
              .format(target, MAX_TARGET), file=sys.stderr)
        target = MAX_TARGET
    size = (target.bit_length() + 7) // 8
    mask64 = 0xffffffffffffffff
    if size <= 3:
        compact = (target & mask64) << (8 * (3 - size))
    else:
        compact = (target >> (8 * (size - 3))) & mask64

    if compact & 0x00800000:
        compact >>= 8
        size += 1

    assert compact == (compact & 0x007fffff)
    assert size < 256
    return compact | size << 24

def bits_to_work(bits):
    return (2 << 255) // (bits_to_target(bits) + 1)

def target_to_hex(target):
    h = hex(target)[2:]
    return '0' * (64 - len(h)) + h

TARGET_1 = bits_to_target(486604799)

INITIAL_BCC_BITS = 403458999
INITIAL_SWC_BITS = 402734313
INITIAL_FX = 0.18
INITIAL_TIMESTAMP = 1503430225
INITIAL_HASHRATE = 500    # In PH/s.
INITIAL_HEIGHT = 481824
INITIAL_SINGLE_WORK = bits_to_work(INITIAL_BCC_BITS)

# Steady hashrate mines the BCC chain all the time.  In PH/s.
STEADY_HASHRATE = 300

# Variable hash is split across both chains according to relative
# revenue.  If the revenue ratio for either chain is at least 15%
# higher, everything switches.  Otherwise the proportion mining the
# chain is linear between +- 15%.
VARIABLE_HASHRATE = 2000   # In PH/s.
VARIABLE_PCT = 15   # 85% to 115%
VARIABLE_WINDOW = 6  # No of blocks averaged to determine revenue ratio

# Greedy hashrate switches chain if that chain is more profitable for
# GREEDY_WINDOW BCC blocks.  It will only bother to switch if it has
# consistently been GREEDY_PCT more profitable.
GREEDY_HASHRATE = 2000     # In PH/s.
GREEDY_PCT = 10
GREEDY_WINDOW = 6

IDEAL_BLOCK_TIME = 10 * 60

State = namedtuple('State', 'height wall_time timestamp bits chainwork fx '
                   'hashrate rev_ratio greedy_frac msg')

states = []


def print_headers():
    print(', '.join(['Height', 'FX', 'Block Time', 'Unix', 'Timestamp',
                     'Difficulty (bn)', 'Implied Difficulty (bn)',
                     'Hashrate (PH/s)', 'Rev Ratio', 'Greedy?', 'Comments']))

def print_state():
    state = states[-1]
    block_time = state.timestamp - states[-2].timestamp
    t = datetime.datetime.fromtimestamp(state.timestamp)
    difficulty = TARGET_1 / bits_to_target(state.bits)
    implied_diff = TARGET_1 / ((2 << 255) / (state.hashrate * 1e15 * IDEAL_BLOCK_TIME))
    print(', '.join(['{:d}'.format(state.height),
                     '{:.8f}'.format(state.fx),
                     '{:d}'.format(block_time),
                     '{:d}'.format(state.timestamp),
                     '{:%Y-%m-%d %H:%M:%S}'.format(t),
                     '{:.2f}'.format(difficulty / 1e9),
                     '{:.2f}'.format(implied_diff / 1e9),
                     '{:.0f}'.format(state.hashrate),
                     '{:.3f}'.format(state.rev_ratio),
                     'Yes' if state.greedy_frac == 1.0 else 'No',
                     state.msg]))

def revenue_ratio(fx, BCC_target):
    '''Returns the instantaneous SWC revenue rate divided by the
    instantaneous BCC revenue rate.  A value less than 1.0 makes it
    attractive to mine BCC.  Greater than 1.0, SWC.'''
    SWC_fees = 0.25 + 2.0 * random.random()
    SWC_revenue = 12.5 + SWC_fees
    SWC_target = bits_to_target(INITIAL_SWC_BITS)

    BCC_fees = 0.2 * random.random()
    BCC_revenue = (12.5 + BCC_fees) * fx

    SWC_difficulty_ratio = BCC_target / SWC_target
    return SWC_revenue / SWC_difficulty_ratio / BCC_revenue

def median_time_past(states):
    times = [state.timestamp for state in states]
    return sorted(times)[len(times) // 2]

def next_bits_k(msg, mtp_window, high_barrier, target_raise_frac,
                low_barrier, target_drop_frac, fast_blocks_pct):
    # Calculate N-block MTP diff
    MTP_0 = median_time_past(states[-11:])
    MTP_N = median_time_past(states[-11-mtp_window:-mtp_window])
    MTP_diff = MTP_0 - MTP_N
    bits = states[-1].bits
    target = bits_to_target(bits)

    # Long term block production time stabiliser
    t = states[-1].timestamp - states[-2017].timestamp
    if t < IDEAL_BLOCK_TIME * 2016 * fast_blocks_pct // 100:
        msg.append("2016 block time difficulty raise")
        target -= target // target_drop_frac

    if MTP_diff > high_barrier:
        target += target // target_raise_frac
        msg.append("Difficulty drop {}".format(MTP_diff))
    elif MTP_diff < low_barrier:
        target -= target // target_drop_frac
        msg.append("Difficulty raise {}".format(MTP_diff))
    else:
        msg.append("Difficulty held {}".format(MTP_diff))

    return target_to_bits(target)

def suitable_block_index(index):
    assert index >= 3
    indices = [index - 2, index - 1, index]

    if states[indices[0]].timestamp > states[indices[2]].timestamp:
        indices[0], indices[2] = indices[2], indices[0]

    if states[indices[0]].timestamp > states[indices[1]].timestamp:
        indices[0], indices[1] = indices[1], indices[0]

    if states[indices[1]].timestamp > states[indices[2]].timestamp:
        indices[1], indices[2] = indices[2], indices[1]

    return indices[1]

def compute_index_fast(index_last):
    for candidate in range(index_last - 3, 0, -1):
        index_fast = suitable_block_index(candidate)
        if index_last - index_fast < 5:
            continue
        if (states[index_last].timestamp - states[index_fast].timestamp
            >= 13 * IDEAL_BLOCK_TIME):
            return index_fast
    raise AssertionError('should not happen')

def compute_target(first_index, last_index):
    work = states[last_index].chainwork - states[first_index].chainwork
    work *= IDEAL_BLOCK_TIME
    work //= states[last_index].timestamp - states[first_index].timestamp
    return (2 << 255) // work - 1

def compute_cw_target(block_count):
    N = len(states) - 1
    last = suitable_block_index(N)
    first = suitable_block_index(N - block_count)
    timespan = states[last].timestamp - states[first].timestamp
    timespan = max(block_count * IDEAL_BLOCK_TIME // 2, min(block_count * 2 * IDEAL_BLOCK_TIME, timespan))
    work = (states[last].chainwork - states[first].chainwork) * IDEAL_BLOCK_TIME // timespan
    return (2 << 255) // work - 1

def next_bits_cw(msg, block_count):
    interval_target = compute_cw_target(block_count)
    return target_to_bits(interval_target)

def next_bits_wt(msg, block_count):
    first, last  = -1-block_count, -1
    timespan = 0
    prior_timestamp = states[first].timestamp
    for i in range(first + 1, last + 1):
        target_i = bits_to_target(states[i].bits)

        # Prevent negative time_i values
        timestamp = max(states[i].timestamp, prior_timestamp)
        time_i = timestamp - prior_timestamp
        prior_timestamp = timestamp
        adj_time_i = time_i * target_i # Difficulty weight
        timespan += adj_time_i * (i - first) # Recency weight

    timespan = timespan * 2 // (block_count + 1) # Normalize recency weight
    target = timespan // (IDEAL_BLOCK_TIME * block_count)
    return target_to_bits(target)

def next_bits_wt_compare(msg, block_count):
    with open("current_state.csv", 'w') as fh:
        for s in states:
            fh.write("%s,%s,%s\n" % (s.height, s.bits, s.timestamp))

    from subprocess import Popen, PIPE

    process = Popen(["./cashwork"], stdout=PIPE)
    (next_bits, err) = process.communicate()
    exit_code = process.wait()

    next_bits = int(next_bits.decode())
    next_bits_py = next_bits_wt(msg, block_count)
    if next_bits != next_bits_py:
        print("ERROR: Bits don't match. External %s, local %s" % (next_bits, next_bits_py))
        assert(next_bits == next_bits_py)
    return next_bits

def next_bits_wtema(msg, alpha_recip):
    # This algorithm is weighted-target exponential moving average.
    # Target is calculated based on inter-block times weighted by a
    # progressively decreasing factor for past inter-block times,
    # according to the parameter alpha.  If the single_block_target SBT is
    # calculated as:
    #    SBT = prior_target * block_time / ideal_block_time
    # then:
    #    next_target = SBT * α + prior_target * (1 - α)
    # Substituting and factorizing:
    #    next_target = prior_target * α / ideal_block_time
    #                  * (block_time + (1 / α - 1) * ideal_block_time)
    # We use the reciprocal of alpha as an integer to avoid floating
    # point arithmetic.  Doing so the above formula maintains precision and
    # avoids overflows wih large targets in regtest
    block_time = states[-1].timestamp - states[-2].timestamp
    prior_target = bits_to_target(states[-1].bits)
    next_target = prior_target // (IDEAL_BLOCK_TIME * alpha_recip)
    next_target *= block_time + IDEAL_BLOCK_TIME * (alpha_recip - 1)
    # Constrain individual target changes to 12.5%
    max_change = prior_target >> 3
    next_target = max(min(next_target, prior_target + max_change),
                      prior_target - max_change)
    return target_to_bits(next_target)

def next_bits_ema(msg, window):
    """This calculates difficulty (1/target) as proportional to the recent hashrate, where "recent hashrate" is estimated by an EMA (exponential moving avg) of recent "hashrate observations", and
    a "hashrate observation" is inferred from each block time.

    Eg, suppose our hashrate estimate before the last block B was H, and thus our difficulty D was proportional to H, intended to yield (on average) a 10-minute block.  But suppose in fact
    block B was mined after only 2 minutes.  Then we infer that during those 2 minutes, hashrate was ~5H, and update our next block's hashrate estimate (and thus difficulty) upwards accordingly.

    In particular, blocks twice as long get twice the weight: a 1-second block tells us hashrate was (probably) high for only 1 second, but a 24-hour block tells us hashrate was (probably) low
    for a full day - the latter *should* get much more weight in our "recent hashrate" estimate."""

    block_time          = states[-1].timestamp - states[-2].timestamp
    block_time          = max(IDEAL_BLOCK_TIME / 100, min(100 * IDEAL_BLOCK_TIME, block_time))          # Crudely dodge problems from ~0/negative/huge block times
    old_hashrate_est    = TARGET_1 / bits_to_target(states[-1].bits)                                    # "Hashrate estimate" - aka difficulty!
    block_weight        = 1 - math.exp(-block_time / window)                                            # Weight of last block_time seconds, according to exp moving avg
    block_hashrate_est  = (IDEAL_BLOCK_TIME / block_time) * old_hashrate_est                            # Eg, if a block takes 2 min instead of 10, we est hashrate was ~5x higher than predicted
    new_hashrate_est    = (1 - block_weight) * old_hashrate_est + block_weight * block_hashrate_est     # Simple weighted avg of old hashrate est, + block's adjusted hashrate est
    new_target          = round(TARGET_1 / new_hashrate_est)
    return target_to_bits(new_target)

def next_bits_ema2(msg, window):
	# A minor reworking of next_bits_ema() above, meant to produce almost exactly the same numbers in typical cases, but be more resilient to huge/0/negative block times.
	max_prev_timestamp = max(state.timestamp for state in states[-100:-1])
	print(max_prev_timestamp)
	block_time = max(min(IDEAL_BLOCK_TIME, window) / 100, states[-1].timestamp - max_prev_timestamp)    # Luckily our target formula is ~flat near 0, so can floor block_time at some small val
	old_target = bits_to_target(states[-1].bits)
	new_target = round(old_target / (1 - math.expm1(-block_time / window) * (IDEAL_BLOCK_TIME / block_time - 1)))
	print(new_target)
	return target_to_bits(new_target)

def ideal(msg, window):
# T=<target solvetime>
	T = IDEAL_BLOCK_TIME

# height -1 = most recently solved block number
# target  = 1/difficulty/2^x where x is leading zeros in coin's max_target, I believe
# Recommended N:

	x = math.log2(math.log2(states[-1].bits))
	N = int((x**x)**0.5)

# To get a more accurate solvetime to within +/- ~0.2%, use an adjustment factor.
# This technique has been shown to be accurate in 4 coins.
# In a formula:
# [edit by zawy: since he's using target method, adjust should be 0.998. This was my mistake. ]
# adjust = 0.9989^(500/N)  
# k = (N+1)/2 * adjust * T 
	k = int((N+1)/2*T)
	sumTarget = 0
	t = 0
	j = 0

# Loop through N most recent blocks.  "< height", not "<=". 
# height-1 = most recently solved rblock
	for i in range(-N, 0, 1):
		solvetime = states[i].timestamp - states[i-1].timestamp
		j += 1
		t += solvetime * j
		sumTarget += bits_to_target(states[i].bits)

# Keep t reasonable in case strange solvetimes occurred. 
	if t < N * k // 3:
		t = N * k // 3

	next_target = t * sumTarget // k // N // N
	return target_to_bits(next_target)

def next_bits_ema_int_approx(msg, window):
    # An integer-math simplified approximation of next_bits_ema2() above.
    max_prev_timestamp = max(state.timestamp for state in states[-100:-1])
    block_time = states[-1].timestamp - max_prev_timestamp 
    old_target = bits_to_target(states[-1].bits)
    new_target = old_target * window // (window + IDEAL_BLOCK_TIME - block_time)                        # Simplifies the corresponding line above via this approx: for 0 <= x << 1, 1-e**(-x) =~ x
    return target_to_bits(new_target)

def exp_int_approx(x, decimals=9):
    """Approximates e**(x / 10**decimals) using integer math, returning the answer scaled by the same number of dec places as the input.  Eg:
    exp_int_approx(1000000, 6) ->  2718281 (e**1 = 2.718281)
    exp_int_approx(3000, 3)    -> 20085    (e**3 = 20.085)
    exp_int_approx(500, 3)     ->  1648    (e**0.5 = 1.648)"""
    assert type(x) is int, str(type(x))                                             # If we pass in a non-int, something has gone wrong

    scaling, scaling_2 = 10**decimals, 10**(2*decimals)
    h = max(0, int.bit_length(x) - int.bit_length(scaling) + 4)                     # h = the number of times we halve x before using our fancy approximation
    term1, term2 = 3 * scaling << h, 3 * scaling_2 << (2*h)                         # Terms from the hairy but accurate approximation we're using - see https://math.stackexchange.com/a/56064
    hth_square_root_of_e_x = scaling_2 * ((x + term1)**2 + term2) // ((x - term1)**2 + term2)

    e_x = hth_square_root_of_e_x                                                    # Now just need to square hth_square_root_of_e_x h times, while repeatedly dividing out our scaling factor
    for i in range(h):
        e_x = e_x**2 // scaling_2
    return e_x // scaling                                                           # And finally, we still have one extra scaling factor to divide out.

def next_bits_ema_int_approx2(msg, window):
    # An integer-math version of next_bits_ema2() above, trying to retain the correct exponential behavior for very long block times.
    max_prev_timestamp = max(state.timestamp for state in states[-100:-1])
    block_time = max(min(IDEAL_BLOCK_TIME, window) // 100, states[-1].timestamp - max_prev_timestamp)
    old_target = bits_to_target(states[-1].bits)
    decimals = 9
    scaling = 10**decimals
    new_target = scaling**2 * old_target // (scaling**2 - (exp_int_approx(scaling * -block_time // window, decimals) - scaling) * (scaling * IDEAL_BLOCK_TIME // block_time - scaling))
    return target_to_bits(new_target)

def next_bits_simple_exponential(msg, window):
    # Dead simple: if the block time is IDEAL_BLOCK_TIME, target is unchanged; if it's more (or less) by n (-n) minutes, scale target by e**(n/window).
    # One nice thing about this is it avoids any need for special handling of huge/0/negative block times.  Eg, successive block times of (-1000000, 1000020) (or vice versa) result in
    # *exactly* the same target as (10, 10).  (This is in fact the only algo with this property!)
    block_time = states[-1].timestamp - states[-2].timestamp
    old_target = bits_to_target(states[-1].bits)
    new_target = round(math.exp((block_time - IDEAL_BLOCK_TIME) / window) * old_target)
    return target_to_bits(new_target)

def next_bits_simple_exponential_int_approx(msg, window):
    # An integer-math version of next_bits_simple_exponential() above.
    block_time = states[-1].timestamp - states[-2].timestamp
    old_target = bits_to_target(states[-1].bits)
    decimals = 9
    scaling = 10**decimals
    new_target = exp_int_approx(scaling * (block_time - IDEAL_BLOCK_TIME) // window, decimals) * old_target // scaling
    return target_to_bits(new_target)

def block_time(mean_time):
    # Sample the exponential distn
    sample = random.random()
    lmbda = 1 / mean_time
    return math.log(1 - sample) / -lmbda

def next_fx_random(r):
    return states[-1].fx * (1.0 + (r - 0.5) / 200)

def next_fx_ramp(r):
    return states[-1].fx * 1.00017149454

def next_step(algo, scenario, fx_jump_factor):
    # First figure out our hashrate
    msg = []
    high = 1.0 + VARIABLE_PCT / 100
    scale_fac = 50 / VARIABLE_PCT
    N = VARIABLE_WINDOW
    mean_rev_ratio = sum(state.rev_ratio for state in states[-N:]) / N
    var_fraction = max(0, min(1, (high - mean_rev_ratio) * scale_fac))
    if ((scenario.pump_144_threshold > 0) and
        (states[-1-144+5].timestamp - states[-1-144].timestamp > scenario.pump_144_threshold)):
        var_fraction = max(var_fraction, .25)

    N = GREEDY_WINDOW
    gready_rev_ratio = sum(state.rev_ratio for state in states[-N:]) / N
    greedy_frac = states[-1].greedy_frac
    if mean_rev_ratio >= 1 + GREEDY_PCT / 100:
        if greedy_frac != 0.0:
            msg.append("Greedy miners left")
        greedy_frac = 0.0
    elif mean_rev_ratio <= 1 - GREEDY_PCT / 100:
        if greedy_frac != 1.0:
            msg.append("Greedy miners joined")
        greedy_frac = 1.0

    hashrate = (STEADY_HASHRATE + scenario.dr_hashrate
                + VARIABLE_HASHRATE * var_fraction
                + GREEDY_HASHRATE * greedy_frac)
    # Calculate our dynamic difficulty
    bits = algo.next_bits(msg, **algo.params)
    target = bits_to_target(bits)
    # See how long we take to mine a block
    mean_hashes = pow(2, 256) // target
    mean_time = mean_hashes / (hashrate * 1e15)
    time = int(block_time(mean_time) + 0.5)
    wall_time = states[-1].wall_time + time
    # Did the difficulty ramp hashrate get the block?
    if random.random() < (abs(scenario.dr_hashrate) / hashrate):
        if (scenario.dr_hashrate > 0):
            timestamp = median_time_past(states[-11:]) + 1
        else:
            timestamp = wall_time + 2 * 60 * 60
    else:
        timestamp = wall_time
    # Get a new FX rate
    rand = random.random()
    fx = scenario.next_fx(rand, **scenario.params)
    if fx_jump_factor != 1.0:
        msg.append('FX jumped by factor {:.2f}'.format(fx_jump_factor))
        fx *= fx_jump_factor
    rev_ratio = revenue_ratio(fx, target)

    chainwork = states[-1].chainwork + bits_to_work(bits)

    # add a state
    states.append(State(states[-1].height + 1, wall_time, timestamp,
                        bits, chainwork, fx, hashrate, rev_ratio,
                        greedy_frac, ' / '.join(msg)))

Algo = namedtuple('Algo', 'next_bits params')

Algos = {
    'k-1' : Algo(next_bits_k, {
        'mtp_window': 6,
        'high_barrier': 60 * 128,
        'target_raise_frac': 64,   # Reduce difficulty ~ 1.6%
        'low_barrier': 60 * 30,
        'target_drop_frac': 256,   # Raise difficulty ~ 0.4%
        'fast_blocks_pct': 95,
    }),
    'cw-144' : Algo(next_bits_cw, {
        'block_count': 144,
    }),
    'wt-144' : Algo(next_bits_wt, {
        'block_count': 144
    }),
    # runs wt-144 in external program, compares with python implementation.
    'wt-144-compare' : Algo(next_bits_wt_compare, {
        'block_count': 144
    }),
    'ema-30min' : Algo(next_bits_ema, { # Exponential moving avg
        'window': 30 * 60,
    }),
    'ema-3h' : Algo(next_bits_ema, {
        'window': 3 * 60 * 60,
    }),
    'ema-1d' : Algo(next_bits_ema, {
        'window': 24 * 60 * 60,
    }),
    'ema2-1d' : Algo(next_bits_ema2, {
        'window': 24 * 60 * 60,
    }),
    'emai-1d' : Algo(next_bits_ema_int_approx, {
        'window': 24 * 60 * 60,
    }),
    'emai2-1d' : Algo(next_bits_ema_int_approx2, {
        'window': 24 * 60 * 60,
    }),
    'wtema-72' : Algo(next_bits_wtema, {
        'alpha_recip': 104, # floor(1/(1 - pow(.5, 1.0/72))), # half-life = 72
    }),
    'wtema-100' : Algo(next_bits_wtema, {
        'alpha_recip': 144, # floor(1/(1 - pow(.5, 1.0/100))), # half-life = 100
    }),
    'simpexp-1d' : Algo(next_bits_simple_exponential, {
        'window': 24 * 60 * 60,
    }),
    'simpexpi-1d' : Algo(next_bits_simple_exponential_int_approx, {
        'window': 24 * 60 * 60,
    }),
    'ideal' : Algo(ideal, {
        'window': 1,
    }),
}

Scenario = namedtuple('Scenario', 'next_fx params, dr_hashrate, pump_144_threshold')

Scenarios = {
    'default' : Scenario(next_fx_random, {}, 0, 0),
    'fxramp' : Scenario(next_fx_ramp, {}, 0, 0),
    # Difficulty rampers with given PH/s
    'dr50' : Scenario(next_fx_random, {}, 50, 0),
    'dr75' : Scenario(next_fx_random, {}, 75, 0),
    'dr100' : Scenario(next_fx_random, {}, 100, 0),
    'pump-osc' : Scenario(next_fx_ramp, {}, 0, 8000),
    'ft100' : Scenario(next_fx_random, {}, -100, 0),
}

def run_one_simul(algo, scenario, print_it):
    states.clear()

    # Initial state is afer 2020 steady prefix blocks
    N = 2020
    for n in range(-N, 0):
        state = State(INITIAL_HEIGHT + n, INITIAL_TIMESTAMP + n * IDEAL_BLOCK_TIME,
                      INITIAL_TIMESTAMP + n * IDEAL_BLOCK_TIME,
                      INITIAL_BCC_BITS, INITIAL_SINGLE_WORK * (n + N + 1),
                      INITIAL_FX, INITIAL_HASHRATE, 1.0, False, '')
        states.append(state)

    # Add 10 randomly-timed FX jumps (up or down 10 and 15 percent) to
    # see how algos recalibrate
    fx_jumps = {}
    factor_choices = [0.85, 0.9, 1.1, 1.15]
    for n in range(10):
        fx_jumps[random.randrange(10000)] = random.choice(factor_choices)

    # Run the simulation
    if print_it:
        print_headers()
    for n in range(10000):
        fx_jump_factor = fx_jumps.get(n, 1.0)
        next_step(algo, scenario, fx_jump_factor)
        if print_it:
            print_state()

    # Drop the prefix blocks to be left with the simulation blocks
    simul = states[N:]

    block_times = [simul[n + 1].timestamp - simul[n].timestamp
                   for n in range(len(simul) - 1)]
    return block_times


def main():
    '''Outputs CSV data to stdout.   Final stats to stderr.'''

    parser = argparse.ArgumentParser('Run a mining simulation')
    parser.add_argument('-a', '--algo', metavar='algo', type=str,
                        choices = list(Algos.keys()),
                        default = 'k-1', help='algorithm choice')
    parser.add_argument('-s', '--scenario', metavar='scenario', type=str,
                        choices = list(Scenarios.keys()),
                        default = 'default', help='scenario choice')
    parser.add_argument('-r', '--seed', metavar='seed', type=int,
                        default = None, help='random seed')
    parser.add_argument('-n', '--count', metavar='count', type=int,
                        default = 1, help='count of simuls to run')
    args = parser.parse_args()

    count = max(1, args.count)
    algo = Algos.get(args.algo)
    scenario = Scenarios.get(args.scenario)
    seed = int(time.time()) if args.seed is None else args.seed

    to_stderr = partial(print, file=sys.stderr)
    to_stderr("Starting seed {} for {} simuls".format(seed, count))

    means = []
    std_devs = []
    medians = []
    maxs = []
    for loop in range(count):
        random.seed(seed)
        seed += 1
        block_times = run_one_simul(algo, scenario, count == 1)
        means.append(statistics.mean(block_times))
        std_devs.append(statistics.stdev(block_times))
        medians.append(sorted(block_times)[len(block_times) // 2])
        maxs.append(max(block_times))

    def stats(text, values):
        if count == 1:
            to_stderr('{} {}s'.format(text, values[0]))
        else:
            to_stderr('{}(s) Range {:0.1f}-{:0.1f} Mean {:0.1f} '
                      'Std Dev {:0.1f} Median {:0.1f}'
                      .format(text, min(values), max(values),
                              statistics.mean(values),
                              statistics.stdev(values),
                              sorted(values)[len(values) // 2]))

    stats("Mean   block time", means)
    stats("StdDev block time", std_devs)
    stats("Median block time", medians)
    stats("Max    block time", maxs)

if __name__ == '__main__':
    main()

