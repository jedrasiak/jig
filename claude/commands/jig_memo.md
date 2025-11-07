---
description: Universal memo command for conversation logging in AsciiDoc format
---

You are running the jig_memo command. This command manages conversation logs in AsciiDoc format.

**Check if arguments were provided after the command:**

## Mode 1: Create New File - /jig_memo <title>

If a title was provided (no file extension):

1. Create a new file named `YYYYMMDD_<sanitized-title>.adoc` (use today's date, sanitize title for filename by replacing spaces with underscores and removing special chars)
2. Use this AsciiDoc template:

```asciidoc
= [Title from argument]
:date: YYYY-MM-DD

== Summary
[Leave blank for now]

```

3. Confirm file creation and remind user to use `/jig_memo <filename>` to save messages

## Mode 2: Update Existing File - /jig_memo <filename>

If a filename was provided (with or without .adoc extension):

1. **Read the conversation file** to get current summary

2. **Save new messages:**
   - Look backwards in current conversation context for the previous `/jig_memo` command
   - Identify all messages AFTER the previous command (or from conversation start if no previous command found) up to and INCLUDING the current `/jig_memo` command
   - Append these messages to the file using format:
     ```asciidoc
     .user
     ----
     [message content]
     ----

     .assistant
     ----
     [message content]
     ----
     ```

3. **Update the summary intelligently:**
   - You have: the OLD summary from file + the NEW messages being appended
   - Analyze how the new messages relate to the conversation's core topic
   - Synthesize an UPDATED summary (2-4 sentences) that reflects:
     - The main thesis/topic of the ENTIRE conversation
     - Current conclusions (which may have evolved from new discussion)
     - Overall direction and key outcomes
   - IMPORTANT: Don't just append "we also talked about X" - if new messages changed conclusions or refined the main topic, the summary should reflect that evolution
   - Replace the content in the `== Summary` section with this synthesized summary

4. Confirm what was saved (e.g., "Saved 4 messages and updated summary")

**Important notes:**
- Never include markdown line number prefixes when reading/writing
- Preserve exact formatting of user and assistant messages
- Handle code blocks, lists, and other formatting within messages correctly
- The `/jig_memo` commands themselves should be saved in the log
- Summary should capture the essence of the WHOLE conversation, not just list topics
