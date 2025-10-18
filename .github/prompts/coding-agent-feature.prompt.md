---
description: Generate GitHub Issue for Coding Agent from feature description
---

# ⚠️ CRITICAL: This prompt uses GitHub MCP Server to create issues automatically

**You MUST use the `mcp_github_create_issue` tool - DO NOT output text for manual creation!**

---

Generate a complete GitHub Issue for the ESP32 Distance Sensor project that will be assigned to the GitHub Copilot Coding Agent.

## Feature Request

${input:featureRequest:Describe the feature you want to implement}

---

## Your Task

Analyze the feature request above and:

1. **Search docs/requirements/** for related REQ-* requirements
2. **Search docs/design/** for related DES-* design elements
3. **Search the codebase** to identify affected components (files/modules)
4. **Determine priority** based on feature type:
   - Core functionality (sensor, LED, WiFi) = High
   - User experience improvements = Medium  
   - Nice-to-have features = Low
5. **Extract clear objective** - what needs to be done and why
6. **Infer current behavior** from codebase context
7. **CREATE THE ISSUE using `mcp_github_create_issue` tool** (owner: "enthali", repo: "esp32-distance")

---

## Issue Structure Template

When creating the issue with `mcp_github_create_issue`, use this structure for the **body** parameter:

## 🎯 Objective

<Rewrite feature request as clear objective - what needs to be done and why>

## 📋 Context

- **Related Requirements**: <List REQ-* IDs found in docs/requirements/, or state "Create REQ-<AREA>-N">
- **Affected Components**: <List files/modules identified from codebase search>
- **Current Behavior**: <Describe current behavior based on code analysis>
- **Desired Behavior**: <Extract from feature request>

## 🔧 Technical Details

<List technical constraints, dependencies, ESP32-specific considerations from your analysis>

---

## MANDATORY WORKFLOW - Follow in Order

### Step 1: Requirements Analysis & Update

1. Read existing requirements in \`docs/requirements/\`
2. Create or update requirements following OpenFastTrack:
   - Format: \`REQ-<AREA>-<NUMBER>\` (e.g., \`REQ-LED-5\`)
   - Include: ID, Title, Description, Rationale, Dependencies, Acceptance Criteria
   - Maintain bidirectional traceability
3. Update \`docs/requirements/<area>-requirements.md\`

### Step 2: Design Documentation

1. Review affected design documents in \`docs/design/\`
2. Document design decisions in code comments with traceability:
   \`\`\`c
   /**
    - @brief Function description
    -
    - DESIGN TRACEABILITY:
    - - DSN-<AREA>-<ID>: Design decision reference
    -
    - REQUIREMENTS TRACEABILITY:
    - - REQ-<AREA>-<ID>: Requirement reference
    */
   \`\`\`
3. Update \`docs/design/<component>-design.md\` if architectural changes

### Step 3: Implementation

Follow ESP32 coding standards from \`.github/prompt-snippets/esp32-coding-standards.md\`:

- Use snake_case for functions/variables
- Include proper error handling with ESP_ERROR_CHECK
- Add ESP_LOG statements for debugging
- Document functions with Doxygen comments
- Add traceability references in code

### Step 4: Quality Gates ⚠️ **CRITICAL - RUN BEFORE COMMIT**

**YOU MUST RUN THESE COMMANDS BEFORE COMMITTING:**

\`\`\`bash
pip install pre-commit mkdocs mkdocs-material markdownlint-cli
markdownlint --fix docs/**/*.md*.md
mkdocs build --strict
pre-commit run --all-files --show-diff-on-failure
\`\`\`

**Why this is critical:**

- CI runs AFTER you commit (too late to fix)
- You will be offline when CI reports errors
- Human maintainer must manually fix your mistakes

### Step 5: Testing

1. Build test: \`idf.py build\`
2. Memory check: \`idf.py size\`
3. Functional testing: Verify feature works as expected
4. Document test results in commit message

### Step 6: Commit with Traceability

Follow commit format from \`.github/prompt-snippets/commit-message.md\`:

\`\`\`
<type>(<scope>): <subject>

<body>

DESIGN TRACEABILITY: DSN-<AREA>-<ID>
REQUIREMENTS: REQ-<AREA>-<ID>
Closes #<issue-number>
\`\`\`

**Types**: feat, fix, docs, refactor, test, chore, perf, security
**Scopes**: sensor, led, wifi, web, build, memory, https, component, requirements, design

---

## ✅ Success Criteria

Before submitting PR verify:

- [ ] Requirements documented and traceable
- [ ] Design documentation updated
- [ ] Code follows ESP32 coding standards
- [ ] All quality gates passed (markdownlint, mkdocs, pre-commit)
- [ ] Build succeeds (\`idf.py build\`)
- [ ] Memory usage acceptable (\`idf.py size\`)
- [ ] Commit messages follow project format
- [ ] Traceability maintained (REQ → DSN → Implementation)

```text

---

## Output Instructions

⚠️ **CRITICAL: USE GITHUB MCP SERVER TO CREATE ISSUE**

**DO NOT** output issue text for manual creation. **ALWAYS** use the GitHub MCP Server tool directly.

### Issue Creation Steps

1. **Analyze the feature request** (search codebase, requirements, determine priority)

2. **Create the issue using `mcp_github_create_issue` tool**:
   - **owner**: `"enthali"`
   - **repo**: `"esp32-distance"`
   - **title**: `"[Feature]: <clear title> #github-pull-request_copilot-coding-agent"`
   - **body**: Complete markdown issue body with all sections (Objective, Context, Technical Details, Workflow Steps, Success Criteria)
   - **labels**: `["feature", "<component>", "coding-agent"]` (e.g., `["feature", "led-controller", "coding-agent"]`)

3. **After creating the issue**, explain your analysis:
   - Which requirements you found (or identified as needed)
   - Which components you identified as affected
   - Priority level and reasoning
   - Any technical considerations discovered
   - Link to the created issue

### Example Tool Usage

```json
{
  "owner": "enthali",
  "repo": "esp32-distance",
  "title": "[Feature]: Implement dual-layer LED animation system #github-pull-request_copilot-coding-agent",
  "body": "## 🎯 Objective\n\n...\n\n## MANDATORY WORKFLOW...",
  "labels": ["feature", "led-controller", "coding-agent"]
}
```

**DO NOT** ask the user to create the issue manually. The GitHub MCP Server will create it automatically.

- ✅ Traceability maintained (REQ → DSN → Implementation)
