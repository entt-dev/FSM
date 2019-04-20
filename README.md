
## Linux Build

### Debug Build

```
cd build
cmake ..
make -j 4
```

### Release Build

```
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j 4
```

For testing and benchmarking
```
make test
make bench
```


## Goals

Most FSM implementations operate on a per-agent basis. At least for me, that meant running the FSM pipeline for every agent (in parallel). This would approximately look something like this:

    for agent in agents:
      state = getState(agent)
      testForNewState(state)
      if state.changed():
        performTransition(agent)

However even if the agents are sorted by state, this will probably lead to a lot of random-access across the ECS registry. To take full advantage of the ECS layout, one can instead operate on a per-state basis. This could then become more like:

    for state in states:
      testForNewStates(state)
      performTransitionsForChangedStates(states)


The cpu likes it when we do things over and over, so the closer we are to running the same test function on the same state component, the higher performance we should expect.
If sets of states are guaranteed to be mutually exclusive to one-another, so that an agent cannot be in two states of the same set, then that means we can parallelize the transition testing of each state.

### Release Benchmarks

* Around 777us for 10,000 agent test-case on 8-core machine when running OMP_NUM_THREADS=1 agents
* Around 537us for 10,000 agent test-case on 8-core machine when running OMP_NUM_THREADS=1 states

So yes, maybe like 30% speed-up on read-heavy operations when running by grouped states vs. switch case on every agent. I suppose this is a good thing
when considering that much of the FSM is inspecting other data and not necessarily writing.

Note: I still think it's too slow for how trivial the transition tests are.


### Thread safety

Useful rules to keep in mind.

Rule 1: When updating the registry during the FSM process, only the agent being tested can be updated. An agent having its state updated cannot update other states at the same time. This is up to the developer to enforce. Only the `EntityState` component should really ever need to be updated. The function objects that are being run however can theoretically be stateful since they're not interacting with anything else.

Rule 2: No signals should be emitted during the parallel sections.
