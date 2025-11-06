# Commit Message Guidelines

## Format

```xml
<type>(<scope>): <subject>

<body>

<footer>
```

## Types

- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code style changes (formatting, etc.)
- **refactor**: Code refactoring without functionality changes
- **test**: Adding or modifying tests
- **chore**: Build process, dependency updates, etc.
- **perf**: Performance improvements
- **security**: Security improvements

## ESP32 Project Specific Scopes

- **component**: Component architecture changes
- **build**: Build system and configuration
- **config**: Configuration management
- **web**: Web server and interface
- **network**: Networking and connectivity
- **memory**: Memory optimization
- **docs**: Documentation updates
- **requirements**: Requirements documentation
- **design**: Design documentation

## Examples

### Feature Addition

```text
feat(config): Implement configuration backup and restore

- Add NVS backup mechanism for configuration data
- Implement restore from backup on corruption detection
- Add validation of restored configuration
- Tested with power loss scenarios
```

### Bug Fix

```text
fix(network): Resolve WiFi reconnection timeout

- Add proper timeout validation for connection attempts
- Implement exponential backoff for retry logic
- Log connection errors appropriately
- Tested with various network conditions
```

### Memory Optimization

```text
perf(build): Optimize flash partition configuration

- Adjust partition table for optimal space usage
- Reduce bootloader size overhead
- Increase available application space
- Verified memory usage with idf.py size
```

### Documentation

```text
docs(requirements): Add system requirements documentation

- Document functional requirements in Sphinx-Needs format
- Add requirement IDs and traceability links
- Include acceptance criteria for each requirement
- Update requirements index
```

## Subject Line Rules

- Use imperative mood ("Add" not "Added")
- No period at the end
- Maximum 50 characters
- Capitalize first letter

## Body Guidelines

- Explain what and why, not how
- Use bullet points for multiple changes
- Reference issue numbers when applicable
- Include testing notes for critical changes

## Footer

- Reference related issues: `Closes #123`
- Note breaking changes: `BREAKING CHANGE: ...`
