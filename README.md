# C TDD Light Scheduler

A C99 implementation of a light-scheduling module, developed with a test-driven workflow.
The scheduler stores recurring light events, obtains the current time through an abstract time
service, and sends ON/OFF commands through a separate light-control interface.

The project uses a mocked time service and a spy implementation of the light-control driver to
test scheduling logic without physical hardware.

## Highlights

- C99 implementation with a fixed in-memory schedule of up to 256 events
- Unit tests built with Unity and CMock
- Mock-based verification of the time-service lifecycle and periodic callback registration
- Spy-based verification of ON/OFF commands sent to the light-control layer
- Saved LCOV report with **100% line coverage** (99/99 lines) and **100% function coverage**
  (19/19 functions) for `src/`

The generated coverage report is available in [`coverage/index.html`](coverage/index.html).

## Architecture

```text
TimeService (mocked in tests)
        |
        | current time + periodic callback
        v
LightScheduler
        |
        | ON / OFF command
        v
LightControlSpy
```

`LightScheduler` owns the scheduling state. `TimeService` provides the current day and minute
of the day, and registers `LightScheduler_wakeUp()` as a 60-second periodic callback.
`LightControlSpy` records the last requested light ID and state, allowing tests to verify the
observable effect of a scheduled event.

## Scheduling model

Each scheduled event contains a light ID from `0` to `255`, a day selector, a minute from
`0` to `1439`, and a requested state: ON or OFF.

Supported day selectors are a specific day from Monday to Sunday, `EVERYDAY`, `WEEKDAY`
(Monday through Friday), and `WEEKEND` (Saturday and Sunday).

At every wake-up, the scheduler retrieves the current time and checks its event table. Matching
events issue either `LightControl_on(id)` or `LightControl_off(id)`.

The public API also supports event removal, querying the current event count, and randomised
ON/OFF scheduling with an offset from -15 to +15 minutes. Randomised times are clamped at
midnight and at the end of the day.

## Validation and error handling

| Condition | Result |
| --- | --- |
| Minute outside `0..1439` | `ERR_INVALID_TIME` |
| Invalid day selector | `ERR_INVALID_DAY` |
| Light ID outside `0..255` | `ERR_INVALID_ID` |
| Conflicting event key | `ERR_CONFLICT` |
| More than 256 scheduled events | `ERR_FULL` |
| Attempt to remove a missing event | `ERR_NOT_FOUND` |

## Test strategy

The test suite starts from externally visible behavior and exercises the implementation through
its interfaces.

### Lifecycle and integration

Tests verify initialization of the time service, registration of a 60-second periodic callback,
event-storage clearing, destruction behavior, repeated destruction calls, re-initialization and
state resetting.

### Scheduling behavior

The suite covers:

- no activity when no event is scheduled;
- triggering at the exact scheduled minute, but not before or after it;
- weekly recurrence;
- specific-day, everyday, weekday and weekend scheduling;
- mirrored ON and OFF behavior;
- multiple lights at the same minute;
- multiple events for the same light at different times;
- conflict detection for the same light, day and minute;
- event removal and reuse of a freed storage slot;
- correct handling of the 256-event capacity.

### Boundary and failure cases

Inputs are tested both just inside and just outside their valid ranges:

- minutes `0` and `1439`, plus invalid negative and overflowing values;
- light IDs `0` and `255`, plus invalid negative and overflowing values;
- valid weekday selectors and invalid enum values;
- removal requests with invalid inputs and missing events.

### Randomised scheduling

Randomised APIs are tested with deterministic `srand()` seeds. The suite verifies that a
predicted random offset is applied, positive and negative offsets are clamped at day boundaries,
invalid inputs are rejected before randomisation, and a randomised time conflicting with an
existing event returns `ERR_CONFLICT`.

## Build and run

Clone the repository with its CMock submodule:

```bash
git clone --recurse-submodules \
  https://github.com/BeatrizAimee/c-tdd-light-scheduler.git
cd c-tdd-light-scheduler
```

Requirements:

- a C99-compatible compiler;
- Ruby;
- LCOV, for coverage generation.

Run the test suite:

```bash
make
```

Generate mocks and run the test targets explicitly:

```bash
make mock_objects
make run_tests
```

Generate a fresh coverage report:

```bash
make coverage
```

This rebuilds `coverage/`. Open `coverage/index.html` locally in a browser to inspect line
and function coverage.

## Repository layout

```text
include/       Public interfaces and scheduler error codes
src/           LightScheduler implementation and LightControl spy
test/          Unity test suites
mocks/         CMock-generated TimeService mock
CMock/         CMock and Unity dependency submodule
coverage/      Saved HTML coverage report
Makefile       Build, test and coverage targets
```

## Notes

The repository intentionally preserves generated mock sources, test executables, raw LCOV data
and the HTML coverage report alongside the source code. They make the saved verification result
inspectable and reproducible with the included build flow.
