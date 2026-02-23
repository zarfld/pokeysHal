# Requirements Prompts Improvements

**Date**: November 6, 2025  
**Issue**: Copilot not consistently delivering expected outputs when executing requirements prompts  
**Solution**: Added explicit "Expected Output" sections with complete templates

---

## 🔍 Root Cause Analysis

### Problems Identified

1. **No Clear Output Format at Top**
   - Deliverable templates were buried deep in the prompt
   - Copilot had to read extensive guidance before understanding what to produce
   - Result: Sometimes stopped after questions, didn't generate final output

2. **Unclear Workflow**
   - Missing "START HERE → DO THIS → DELIVER THAT" structure
   - No explicit steps showing what to do when
   - Result: Copilot unclear about sequence of actions

3. **Example-Heavy Content**
   - Many examples but unclear which was the actual deliverable
   - Guidance overwhelmed the task definition
   - Result: Copilot focused on examples rather than producing output

4. **Missing Explicit Instructions**
   - No "ALWAYS DELIVER" or "MUST PRODUCE" language
   - Lacked emphasis on completing the full deliverable
   - Result: Partial outputs or summaries instead of complete documents

---

## ✅ Solutions Implemented

### 1. Added "📤 EXPECTED OUTPUT" Section

**Location**: Immediately after the agent role definition (lines 10-80 typically)

**Format**:
```markdown
## 📤 EXPECTED OUTPUT (ALWAYS DELIVER)

When [trigger condition], you **MUST** produce:

[Complete template showing exactly what to generate]
```

**Benefits**:
- ✅ Copilot sees expected output format first
- ✅ Clear template to follow
- ✅ No ambiguity about deliverables

### 2. Added "🎯 Your Task" Section

**Location**: After expected output, before detailed guidance

**Format**:
```markdown
## 🎯 Your Task

When [condition], you will:

**STEP 1**: [Action with reference to output]
**STEP 2**: [Action]
**STEP 3**: [Action]
**STEP 4**: Generate complete [deliverable] (see Expected Output above)

**Always deliver [output type]!** Do not skip or summarize.
```

**Benefits**:
- ✅ Clear step-by-step workflow
- ✅ References back to expected output
- ✅ Explicit instruction to complete full deliverable

### 3. Restructured Prompt Flow

**New Structure**:
```
1. YAML front matter
2. Role definition
3. 📤 EXPECTED OUTPUT (with complete template)
4. 🎯 Your Task (step-by-step workflow)
5. Detailed guidance (reference material)
```

**Old Structure**:
```
1. YAML front matter
2. Role definition
3. Objectives
4. Detailed guidance
5. Examples
6. Usage (deliverable buried here)
```

**Benefits**:
- ✅ Critical information first
- ✅ Reference material last (for context)
- ✅ Copilot knows what to produce before reading examples

---

## 📋 Files Updated

### 1. requirements-elicit.prompt.md

**Changes**:
- ✅ Added explicit output template for clarifying questions document
- ✅ Added template for complete requirements specification
- ✅ Added 3-step workflow (questions → answers → specification)
- ✅ Emphasized "Always deliver both outputs!"

**Expected Output**:
1. Clarifying Questions Document (8 dimensions)
2. Complete Requirements Specification (after stakeholder answers)

### 2. requirements-complete.prompt.md

**Changes**:
- ✅ Added complete audit report template with scorecards
- ✅ Added explicit sections: Executive Summary, Critical Gaps, Action Items, Scorecards
- ✅ Added 4-step workflow with clear deliverable
- ✅ Emphasized "Always deliver the full report!"

**Expected Output**:
- Complete Requirements Completeness Audit Report
- Scored across 10 dimensions per requirement
- Action items prioritized (P0/P1/P2)

### 3. requirements-refine.prompt.md

**Changes**:
- ✅ Added refinement report template with before/after comparison
- ✅ Added quality assessment scorecard (8 characteristics)
- ✅ Added refined requirement template (ready to use)
- ✅ Added 4-step workflow
- ✅ Emphasized "Always deliver the full report!"

**Expected Output**:
- Requirement Refinement Report
- Quality scores (0-80 scale)
- Clarifying questions
- Refined requirement (copy-ready)

### 4. requirements-validate.prompt.md

**Changes**:
- ✅ Added complete validation report template
- ✅ Added 6 validation types with detailed results sections
- ✅ Added traceability matrix format
- ✅ Added compliance metrics table
- ✅ Added 5-step workflow
- ✅ Emphasized "Always deliver the full report!"

**Expected Output**:
- Requirements Validation Report
- ISO 29148:2018 compliance checking
- Traceability matrix
- Prioritized recommendations

---

## 🎯 Key Improvements Summary

| Aspect | Before | After |
|--------|--------|-------|
| **Output Format** | Buried in examples | First thing shown with template |
| **Workflow** | Implicit | Explicit steps |
| **Completeness** | Sometimes partial | "MUST produce" + full template |
| **Template** | Scattered examples | One complete copy-ready template |
| **Emphasis** | None | Bold "Always deliver!" |

---

## 📊 Expected Results

### Before Improvements
```
User: "Check requirements completeness"
Copilot: 
  "I'll check the requirements...
   - Missing error handling in REQ-F-001
   - Missing acceptance criteria in REQ-F-003
   
   Let me know if you need more details."
   
❌ No structured report
❌ No scores
❌ No action items
```

### After Improvements
```
User: "Check requirements completeness"
Copilot:
  # Requirements Completeness Audit Report
  
  **Overall Completeness Score**: 78%
  **Status**: ⚠️ NEEDS WORK
  
  | Dimension | Score | Status |
  |-----------|-------|--------|
  | Functional | 9/10 | ✅ |
  | Error Handling | 3/10 | 🔴 |
  ...
  
  ## Critical Gaps
  
  ### REQ-F-001
  Score: 45/100
  Missing:
  - ❌ Error handling scenarios
  - ❌ Acceptance criteria
  
  Fixes:
  1. Add error table...
  
  [COMPLETE REPORT WITH ALL SECTIONS]
  
✅ Structured report
✅ Scores for all dimensions
✅ Prioritized action items
✅ Ready to act on
```

---

## 🚀 Usage Guidelines

### For Users

**Before executing a prompt**:
1. Read the "📤 EXPECTED OUTPUT" section
2. Know what you'll receive
3. Prepare any inputs needed (files, requirement text, etc.)

**During execution**:
1. Copilot will follow the "🎯 Your Task" steps
2. Wait for complete output (may take time for large reports)
3. Don't interrupt early - let it complete all sections

**After receiving output**:
1. Verify all sections present (check against template)
2. If incomplete, ask: "Please complete all sections from the Expected Output template"
3. Use the output as-is or customize for your needs

### For Prompt Authors

**When creating new prompts**:
1. ✅ Start with "📤 EXPECTED OUTPUT" section showing complete template
2. ✅ Add "🎯 Your Task" with explicit steps
3. ✅ Use "**MUST**", "**ALWAYS DELIVER**", "Do not skip" language
4. ✅ Reference expected output in task steps ("see Expected Output above")
5. ✅ Put detailed guidance AFTER expected output
6. ✅ Test prompt to ensure full deliverable is generated

---

## 🧪 Testing Recommendations

### Test Each Prompt

1. **requirements-elicit.prompt.md**
   ```
   Test: "Help me write requirements for user authentication"
   Expected: 
   - Clarifying questions (8 dimensions)
   - After answers: Complete requirement specification
   ```

2. **requirements-complete.prompt.md**
   ```
   Test: "Check completeness of REQ-F-001"
   Expected:
   - Full audit report with scores for all 10 dimensions
   - Critical gaps listed
   - Action items prioritized
   ```

3. **requirements-refine.prompt.md**
   ```
   Test: "Refine this requirement: The system shall be fast"
   Expected:
   - Quality assessment (8 characteristics scored)
   - Clarifying questions
   - Refined requirement with specifics
   ```

4. **requirements-validate.prompt.md**
   ```
   Test: "Validate all requirements in ./02-requirements/"
   Expected:
   - Complete validation report
   - 6 validation types checked
   - Traceability matrix
   - Compliance scores
   ```

### Success Criteria

✅ Each prompt produces complete output matching template  
✅ No partial outputs or summaries  
✅ All sections present  
✅ Output is copy-ready (can use as-is)  
✅ Consistent format across multiple runs  

---

## 📚 Related Documentation

- [Root Copilot Instructions](../.github/copilot-instructions.md) - Overall guidance
- [Phase 02 Instructions](../instructions/phase-02-requirements.instructions.md) - Requirements phase guidance
- [Lifecycle Guide](../../docs/lifecycle-guide.md) - Complete lifecycle walkthrough

---

## 🔄 Version History

- **v2.0** (2025-11-06): Major restructuring with explicit output templates
- **v1.0** (Initial): Original prompts with embedded guidance

---

## 💡 Tips for Best Results

1. **Be Specific in Your Request**
   ```
   ❌ "Check requirements"
   ✅ "Run completeness check on REQ-F-001 through REQ-F-010"
   ```

2. **Reference Files Explicitly**
   ```
   ❌ "Validate requirements"
   ✅ "Validate requirements in ./02-requirements/functional/auth.md"
   ```

3. **Wait for Complete Output**
   - Copilot may take 30-60 seconds for full reports
   - Don't interrupt early
   - If output stops, ask: "Please complete the remaining sections"

4. **Provide Context When Needed**
   ```
   For elicitation:
   "Elicit requirements for [feature] targeting [audience] with [constraints]"
   
   For validation:
   "Validate against ISO 29148:2018 for [compliance level]"
   ```

---

**Result**: Requirements prompts now consistently deliver complete, structured outputs ready for immediate use! 🚀
