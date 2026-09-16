# Security Policy

## Supported Versions

We release patches for security vulnerabilities. Which versions are eligible
for receiving such patches depends on the CVSS v3.0 rating:

| Version | Supported          |
| ------- | ------------------ |
| latest  | :white_check_mark: |
| < latest| :x:                |

## Reporting a Vulnerability

We take the security of FunctionPro seriously. If you believe you have found a
security vulnerability, please report it to us responsibly.

**Please do not report security vulnerabilities through public GitHub issues.**

Instead, please report them via one of the following methods:

- **GitHub Security Advisories**: Use the
  ["Report a vulnerability"](https://github.com/privateMwb/FunctionPro/security/advisories/new)
  button in this repository's Security tab. This creates a private
  discussion with maintainers and is the only supported reporting channel
  for this project.

Please include as much of the following information as possible:

- Type of vulnerability (e.g., buffer overflow, injection, etc.)
- Full paths of affected source file(s)
- Location of the affected code (tag/branch/commit or direct URL)
- Any special configuration required to reproduce the issue
- Step-by-step instructions to reproduce the issue
- Proof-of-concept or exploit code (if possible)
- Impact of the issue, including how an attacker might exploit it

## Disclosure Process and Timelines

- We will acknowledge receipt of your vulnerability report within **3
  business days**.
- We will provide an initial assessment of the report within **10 business
  days**, including whether it is confirmed as a vulnerability and its
  severity.
- We aim to release a fix within **90 days** of confirming a vulnerability,
  depending on complexity and severity. Critical vulnerabilities will be
  prioritized for faster resolution.
- We will coordinate with the reporter on public disclosure timing. We
  follow a **coordinated disclosure** model: details are not published until
  a fix is available, or 90 days have elapsed, whichever comes first.
- Credit will be given to the reporter in the release notes and security
  advisory, unless anonymity is requested.

## Scope

This policy applies to the code contained in this repository. Vulnerabilities
in third-party dependencies should be reported to the respective upstream
projects, though we welcome reports here as well so we can track and update
affected dependencies.

Thank you for helping keep FunctionPro and its users safe.
