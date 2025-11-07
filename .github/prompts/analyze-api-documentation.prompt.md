# Validate API Documentation in Header File

## Quick Usage

```text
Validate API documentation for: [HEADER_FILE_PATH]
Output analysis to: tools/temp/[module_name]_doc_analysis.md
```

## Purpose

Analyze a C header file to verify it contains complete, high-quality documentation suitable for generating Sphinx-Needs API docs. Generate a detailed analysis report with findings and recommendations.

## Instructions

1. **Read the header file** and analyze:
   - File-level documentation (@file, @brief, @author, version, architecture notes)
   - Function documentation completeness
   - Struct/enum documentation
   - Parameter descriptions
   - Return value documentation
   - Requirement traceability (REQ_* mentions)
   - Usage examples and notes

2. **Check for Documentation Quality Issues:**

   **File-Level (CRITICAL):**
   - [ ] Has `@file` tag with filename
   - [ ] Has `@brief` with clear component description
   - [ ] Contains architecture/design notes
   - [ ] Lists key features or usage patterns
   - [ ] Mentions requirement IDs (REQ_*)
   - [ ] Has author/version information

   **Function-Level (MANDATORY):**
   - [ ] Every public function has a documentation block
   - [ ] Function purpose is clearly described
   - [ ] All parameters are documented with @param or in description
   - [ ] Return values are documented with @return or explanation
   - [ ] Error codes are enumerated (ESP_OK, ESP_ERR_*)
   - [ ] Thread-safety notes if applicable
   - [ ] Usage constraints or special requirements mentioned

   **Data Types (if present):**
   - [ ] Structs have descriptions of their purpose
   - [ ] Struct fields are documented (inline or block comments)
   - [ ] Enums have value descriptions
   - [ ] Typedefs explain the abstraction

   **Traceability:**
   - [ ] Requirement IDs mentioned in relevant places
   - [ ] Links to design specifications if applicable

3. **Generate Analysis Report** in Markdown format:

```markdown
# API Documentation Analysis: [Module Name]

**Header File:** `[path/to/header.h]`  
**Analysis Date:** [YYYY-MM-DD]  
**Status:** ✅ PASS / ⚠️ NEEDS IMPROVEMENT / ❌ FAIL

---

## Executive Summary

Brief overview of documentation quality (2-3 sentences).

**Overall Score:** [X/100]

**Key Findings:**
- ✅ [Major strength]
- ⚠️ [Issue that needs attention]
- ❌ [Critical missing element]

---

## File-Level Documentation

**Status:** ✅ Complete / ⚠️ Partial / ❌ Missing

### Present Elements:
- ✅ @file tag: `config_manager.h`
- ✅ @brief: "JSON Schema-Driven Configuration Management"
- ✅ Architecture notes: [summary of architecture section]
- ✅ Key features listed: [count] features documented

### Missing/Weak Elements:
- ❌ No @author tag
- ⚠️ Usage examples could be more detailed

### Requirements Traceability:
- ✅ Found [X] requirement references: REQ_CFG_JSON_1, REQ_CFG_JSON_7...
- ❌ No links to design documents

---

## Function Documentation Analysis

**Total Functions:** [X]  
**Fully Documented:** [Y] ([percentage]%)  
**Partially Documented:** [Z]  
**Undocumented:** [N]

### Compliance Matrix:

| Function | Purpose | Params | Returns | Thread-Safety | REQ Links | Status |
|----------|---------|--------|---------|---------------|-----------|--------|
| `config_init()` | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ PASS |
| `config_get_string()` | ✅ | ✅ | ✅ | ❌ | ✅ | ⚠️ PARTIAL |
| `config_set_bool()` | ❌ | ⚠️ | ❌ | ❌ | ❌ | ❌ FAIL |

### Detailed Findings:

#### ✅ Well-Documented Functions:
1. **`config_init()`**
   - Clear purpose statement
   - No parameters (explicitly noted)
   - All return codes documented
   - Thread-safety mentioned
   - Links to REQ_CFG_JSON_12

#### ⚠️ Partially Documented Functions:
1. **`config_get_string()`**
   - Missing: Thread-safety notes
   - Missing: Buffer size recommendations
   - Present: All parameters, return codes

#### ❌ Poorly/Undocumented Functions:
1. **`config_set_bool()`**
   - Missing: Function purpose description
   - Missing: Return value documentation
   - Missing: Parameter descriptions
   - Present: Function signature only

---

## Data Types Documentation

**Total Structs:** [X] | **Documented:** [Y]  
**Total Enums:** [X] | **Documented:** [Y]

### Struct Analysis:

#### `wifi_status_t` - ⚠️ PARTIAL
- ✅ Struct purpose documented
- ⚠️ Only 3/5 fields have descriptions
- ❌ Missing usage example

#### `config_credentials_t` - ❌ MISSING
- ❌ No struct-level documentation
- ❌ No field descriptions

### Enum Analysis:

#### `wifi_mode_t` - ✅ COMPLETE
- ✅ Enum purpose clear
- ✅ All 5 values documented with inline comments

---

## Requirements Traceability Analysis

**Total REQ_* References:** [X]

**Referenced Requirements:**
- REQ_CFG_JSON_1 - JSON Schema as Source of Truth
- REQ_CFG_JSON_7 - Type-Safe Configuration API
- REQ_CFG_JSON_12 - Configuration Lifecycle

**Coverage:**
- ✅ File-level traceability: [X] requirements
- ✅ Function-level traceability: [Y] functions linked
- ⚠️ Unlinked functions: [Z] functions missing requirement links

**Orphan Functions** (no REQ links):
1. `config_commit()` - Should link to REQ_CFG_JSON_X
2. `config_get_int16()` - Should link to REQ_CFG_JSON_7

---

## Code Examples & Usage Patterns

**Status:** ✅ Present / ⚠️ Limited / ❌ Missing

### Found Examples:
- ✅ File-level usage workflow (4-step pattern)
- ❌ No function-level usage examples
- ❌ No error handling examples

### Recommendations:
1. Add usage example for `config_init()` error handling
2. Add thread-safety example for concurrent access patterns
3. Document typical call sequences

---

## Documentation Quality Metrics

| Metric | Score | Target | Status |
|--------|-------|--------|--------|
| File-level completeness | 85% | 100% | ⚠️ |
| Function coverage | 95% | 100% | ⚠️ |
| Parameter documentation | 100% | 100% | ✅ |
| Return value documentation | 90% | 100% | ⚠️ |
| Thread-safety notes | 60% | 80% | ❌ |
| Requirements traceability | 75% | 90% | ⚠️ |
| Code examples | 20% | 50% | ❌ |
| **Overall Score** | **75/100** | **85+** | ⚠️ |

---

## Recommendations

### High Priority (Critical for API Doc Generation):
1. ❌ **Add missing function descriptions** for `config_set_bool()`, `config_get_int32()`
2. ❌ **Document all return values** - 3 functions missing ESP_ERR_* codes
3. ⚠️ **Add thread-safety notes** for all multi-task-safe functions

### Medium Priority (Quality Improvements):
4. ⚠️ Complete struct field documentation for `wifi_status_t`
5. ⚠️ Add requirement links for orphaned functions
6. ⚠️ Include @author and @version tags

### Low Priority (Nice to Have):
7. ℹ️ Add usage examples for complex APIs
8. ℹ️ Document error handling patterns
9. ℹ️ Add cross-references between related functions

---

## Readiness for API Doc Generation

**Can generate docs now?** ✅ YES / ⚠️ WITH ISSUES / ❌ NOT READY

**Blockers:**
- [List any critical issues preventing doc generation]

**Warnings:**
- [List quality issues that will result in incomplete docs]

**Recommendation:**
[Overall recommendation: ready to generate, needs minor fixes, or requires major documentation work]

---

## Next Steps

1. [ ] Address high-priority issues listed above
2. [ ] Re-run validation to confirm fixes
3. [ ] Generate API documentation using `api-doc-generation.prompt.md`
4. [ ] Review generated RST for completeness
```

4. **Save the report** to:
   - Path: `tools/temp/[module_name]_doc_analysis.md`
   - Example: `tools/temp/config_manager_doc_analysis.md`

## Scoring Guidelines

**Overall Score Calculation:**
- File-level completeness: 20 points
- Function documentation: 40 points
- Data types documentation: 15 points
- Requirements traceability: 15 points
- Code examples: 10 points

**Status Determination:**
- **90-100**: ✅ EXCELLENT - Ready for doc generation
- **75-89**: ⚠️ GOOD - Minor improvements recommended
- **60-74**: ⚠️ NEEDS WORK - Several issues to address
- **Below 60**: ❌ POOR - Major documentation gaps

## Quality Standards

**For PASS status, minimum requirements:**
- File-level @file and @brief present
- 90%+ functions have purpose descriptions
- 90%+ parameters documented
- 80%+ return values documented
- 50%+ functions have requirement links

**For EXCELLENT status:**
- All above metrics at 100%
- Thread-safety notes where applicable
- Usage examples present
- Comprehensive traceability

## Example Invocation

```text
Validate API documentation for: main/components/config_manager/config_manager.h
Output analysis to: tools/temp/config_manager_doc_analysis.md
```

## Notes

- This validation checks the **source header file**, not the generated RST
- Use this before running `api-doc-generation.prompt.md` to ensure quality
- Re-run after fixing documentation issues to verify improvements
- Analysis report is for human review, not automated processing
