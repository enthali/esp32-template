# Browser-Based Development Access

This development container supports browser-based access for both local and cloud development.

## Access Methods

### Local Development (Docker on Windows/Mac/Linux)

When running the dev container locally, you have **two** browser access options:

#### 1. VS Code in Browser (Port 8088)

- **URL**: `http://localhost:8088`
- **What**: Full VS Code interface in your browser
- **Use case**: Work from any device without VS Code Desktop installed
- **Auto-starts**: Yes, automatically when container starts (local only)

#### 2. Desktop Environment (Port 6080)

- **URL**: `http://localhost:6080`
- **What**: Fluxbox desktop environment via noVNC
- **Use case**: Run GUI applications (e.g., `gtkwave`, graphical tools)
- **Auto-starts**: Yes, always available

### GitHub Codespaces

When running in GitHub Codespaces:

- **VS Code**: Access via `https://<codespace-name>.github.dev` (built-in)
- **Desktop**: Access via forwarded port 6080
- **code-server**: Not started (redundant with Codespaces)

## How It Works

### Automatic Detection

The startup script (`tools/on-start.sh`) automatically detects the environment:

```bash
if [ -z "$CODESPACES" ]; then
    # Local: Start code-server
    code-server --bind-addr 0.0.0.0:8088 ...
else
    # Codespaces: Skip (already have VS Code Server)
fi
```

### Port Forwarding

The `devcontainer.json` forwards these ports:

```json
"forwardPorts": [6080, 8088],
"portsAttributes": {
    "6080": { "label": "noVNC (Desktop)" },
    "8088": { "label": "code-server (VS Code Browser)" }
}
```

## Starting the Container

### From VS Code Desktop

1. Open project in VS Code
2. Command: "Dev Containers: Reopen in Container"
3. Wait for container to build/start
4. Access:
   - VS Code Desktop: Already connected
   - VS Code Browser: `http://localhost:8088`
   - Desktop GUI: `http://localhost:6080`

### Standalone (Docker Command)

```bash
# Build the container
docker build -t esp32-template-dev .devcontainer

# Run with ports exposed
docker run -it --rm \
  -v $(pwd):/workspaces/esp32-template \
  -p 6080:6080 \
  -p 8088:8088 \
  --privileged \
  esp32-template-dev

# Access in browser
# - http://localhost:8088 (VS Code)
# - http://localhost:6080 (Desktop)
```

## Security Considerations

### Authentication

By default, **no authentication** is configured for ease of local development:

```bash
code-server --auth none  # ⚠️ Local only!
```

**For remote access**, enable password authentication:

```bash
# In tools/on-start.sh, change:
code-server --auth password --password "your-secure-password" ...
```

### Network Access

- **Local**: Binds to `0.0.0.0` (all interfaces)
- **Docker**: Ports only accessible from host machine
- **Codespaces**: GitHub handles authentication automatically

## Troubleshooting

### code-server not starting

Check logs:

```bash
cat /tmp/code-server.log
```

Verify it's running:

```bash
ps aux | grep code-server
```

### Port already in use

Check what's using the port:

```bash
sudo netstat -tuln | grep 8088
```

Kill existing process:

```bash
pkill -f code-server
```

### Can't access from browser

1. Verify port forwarding in VS Code (Ports panel)
2. Check firewall settings on host machine
3. Ensure Docker port mapping is correct

## Performance Considerations

### Resource Usage

Running code-server adds:

- **Memory**: ~200-300 MB
- **CPU**: Minimal when idle
- **Disk**: ~150 MB (installed)

### When to Use

✅ **Use code-server when:**

- Working from tablet or non-development machine
- Need to share development environment quickly
- Want consistent experience across devices
- Running container on remote server

❌ **Stick with VS Code Desktop when:**

- Working on your primary development machine
- Need best performance
- Using VS Code extensions not supported in browser

## Advanced Configuration

### Custom Port

Change port in `tools/on-start.sh`:

```bash
code-server --bind-addr 0.0.0.0:9999 ...
```

And update `devcontainer.json`:

```json
"forwardPorts": [6080, 9999]
```

### Extensions

code-server automatically syncs extensions from:

- `~/.local/share/code-server/extensions/`

Install extensions via command line:

```bash
code-server --install-extension ms-python.python
```

### Theme and Settings

Settings persist in:

- `~/.local/share/code-server/User/settings.json`

## Related Documentation

- [noVNC Setup](../README.md#desktop-environment)
- [QEMU Network Access](../README.md#qemu-emulation)
- [Dev Container Configuration](../../.devcontainer/README.md)
