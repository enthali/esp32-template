# Development Workflow

This document outlines the essential development workflow for ESP32 projects in this template.

## Branch Strategy

### Main Branches

- `main`: Stable release branch
- `develop`: Development integration branch

### Feature Branches

Use descriptive branch names:

- `feat/component-name`
- `fix/issue-description`
- `docs/update-topic`

## Pull Request Process

1. Create feature branch from `develop`
2. Implement changes following coding standards
3. Run pre-commit checks: `pre-commit run --all-files`
4. Submit PR with clear description
5. Address review feedback
6. Merge after approval
