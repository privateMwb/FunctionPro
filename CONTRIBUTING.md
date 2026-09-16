# Contributing to FunctionPro

Thanks for your interest in contributing! This document explains how the
project accepts changes.

## Reporting bugs and requesting features

Please use [GitHub Issues](../../issues) for bug reports and feature
requests. When filing a bug, include:

- What you expected to happen vs. what actually happened
- Steps to reproduce (a minimal code sample helps a lot)
- Your compiler, OS, and FunctionPro version/commit

## Contributing changes

1. **Fork the repository** and create a branch off `main` for your change.
2. **Make your change**, following the existing code style. This project
   uses `clang-format` and `clang-tidy`; running them locally before
   opening a PR will save round-trips:
   ```
   clang-format -i <changed files>
   clang-tidy <changed files>
   ```
3. **Add or update tests** where relevant. Tests are built with
   `-DBUILD_TESTS=ON` and run as part of CI.
4. **Open a pull request** against `main`. Please describe what the change
   does and why.
5. **CI must pass.** Every PR runs the build, clang-format check,
   clang-tidy, sanitizers, and coverage workflows automatically. PRs with
   failing checks won't be merged.
6. **Review.** This project currently has a single maintainer. Pull
   requests from external contributors are reviewed and merged by the
   maintainer once CI passes and the change looks good. (Note: the
   maintainer merges their own commits via a scoped bypass rule, since
   GitHub doesn't allow self-approval — external contributions go through
   the same review scrutiny regardless.)

## Coding style

- Match the formatting enforced by the project's `.clang-format` config.
- Prefer clear, well-named code over clever one-liners.
- Keep public API changes backward-compatible where possible; call out
  breaking changes explicitly in your PR description.

## License

By contributing, you agree that your contributions will be licensed under
the same license as the rest of the project (see `LICENSE`).
