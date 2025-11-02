# code-server Integration

## What is code-server?

code-server is VS Code running in your browser. It enables access to your development environment from any device with a web browser.

## Why local only?

- **GitHub Codespaces**: Already provides VS Code Server (redundant)
- **Local Development**: Enables browser access to the Dev Container
- **Automatic Detection**: Only starts when `$CODESPACES` is not set

## Access

### Local (Docker)

```bash
# Start container (e.g., via VS Code)
# Then open in browser:
http://localhost:8088
```

### GitHub Codespaces

```bash
# No code-server needed
# Use the normal Codespaces URL
https://<name>.github.dev
```

## Configuration

### Startup

Startup happens in `tools/on-start.sh`:

```bash
if [ -z "$CODESPACES" ]; then
    code-server --bind-addr 0.0.0.0:8088 --auth none ...
fi
```

### Port Forwarding

Configured in `.devcontainer/devcontainer.json`:

```json
"forwardPorts": [8088],
"portsAttributes": {
    "8088": { "label": "code-server (VS Code Browser)" }
}
```

## Security

**Local**: No authentication (`--auth none`)

**Remote**: For remote access, enable password authentication:

```bash
code-server --auth password --password "your-secure-password"
```

## Logs

```bash
# View logs
cat /tmp/code-server.log

# Check process
ps aux | grep code-server
```

## More Information

See [Browser Access Guide](../docs/90_guides/browser-access.md) for details.
