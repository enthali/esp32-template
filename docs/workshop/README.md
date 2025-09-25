
# Workshop: Embedded GitHub Coding (Materials)

This section contains the workshop materials. The goal is to give participants hands-on experience working on open-source embedded projects (ESP32 + HC-SR04 + WS2812) using GitHub, Codespaces and Copilot/Coding-Agents.

Included files (short overview):

- `WORKSHOP-BRAINSTORMING.md` — agenda, goals and schedule
- `COPILOT-PROMPTS.md` — example prompts for Copilot / coding agents
- `CODESPACES-GUIDE.md` — step-by-step guide to open a Codespace / Dev Container
- `BUILD-FLASH-INSTRUCTIONS.md` — build and flash instructions (ESP-IDF)
- `ISSUE-MCP-EXERCISE.md` — exercise: create an issue & use a coding agent

## Prerequisites

- Git & GitHub access
- VS Code (recommended) with Dev Container / Codespaces or a local ESP-IDF setup
- USB cable and an ESP32 development board for flashing (optional for hands-on)

## Quick flow

1. Verify setup (Codespace or local Dev Container)
2. Introduce repository structure and components
3. Small tasks/issues: measure, display, extend LED patterns
4. Use Copilot/Coding-Agents for assistance
5. Code review, tests, merge process and wrap-up

## Example workshop exercises

- Exercise A: Extend LED patterns (scoring: readability & memory efficiency)
- Exercise B: Make distance measurement more robust (timeouts, filtering)
- Exercise C: Test HTTPS web interface (certificates in the build)

## FAQ / Troubleshooting (short)

Q: Codespaces not available?

A: Use local VS Code with the included Dev Container, or follow `CODESPACES-GUIDE.md`.

Q: Copilot returns unhelpful suggestions?

A: Provide more context in the prompt (e.g. REQ IDs, file paths, expected return values). Always review suggestions — never accept blindly.

Q: The coding agent made incorrect changes?

A: Stop the agent, inspect the branch, and fix changes manually. Use PR review workflow (branch → PR → review → merge).

## Contribution & Next Steps

- Before pushing changes: follow the commit message format (see `/.github/prompt-snippets/commit-message.md`).
- Suggested first local commit (after edits):

```
git checkout -b workshop
git add workshop/README.md
git commit -m "docs(workshop): update workshop README (EN)"
```

- Optional: open a PR from `workshop` → `main` and request review.

---

These files were added as a starting point for workshop materials and translated/extended.
