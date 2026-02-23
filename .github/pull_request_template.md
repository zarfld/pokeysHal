## Description

Brief description of the changes in this PR.

## Related Issues

Fixes #
Implements #
Part of #

## Type of Change

- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update
- [ ] Real-time / HAL component change

## Traceability

- **Requirements**: #
- **Architecture / Design**: #
- **Tests**: #

## Testing

Describe how this was tested:

- [ ] `bash test_compile.sh` passes
- [ ] `make -f Makefile.noqmake` builds without errors
- [ ] HAL userspace component loads (`halrun` / `loadusr`)
- [ ] HAL RT component loads (`halrun` / `loadrt`)
- [ ] No memory allocation in RT paths
- [ ] No blocking calls in RT paths

## Real-Time Safety Checklist (if RT code changed)

- [ ] No `malloc`/`free` in real-time thread paths
- [ ] No blocking socket operations (all sockets are `O_NONBLOCK`)
- [ ] RT function completes within timing budget (<50µs for PREEMPT_RT userspace/soft-RT; <5µs for RTAI/hard-RT kernel module)
- [ ] `mlockall` used where required
- [ ] HAL pin types are correct (`hal_s32_t`, `hal_bit_t`, `hal_float_t`)

## Checklist

- [ ] All tests pass
- [ ] Documentation updated alongside code changes
- [ ] No unrelated files changed
- [ ] Traceability links verified (PR links to implementing issue)
