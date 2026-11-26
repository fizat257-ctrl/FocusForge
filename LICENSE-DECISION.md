# FocusForge License Decision Note

**Status: No software license selected.** This note describes options for the project owner; it is not a license grant and does not replace a `LICENSE` file, EULA, or legal advice. Do not treat the current generated CPack installer text (“No license provided”) as FocusForge licensing terms.

## If Retaining Control Is the Priority

### Proprietary, All Rights Reserved

The owner retains control and grants users only the permissions stated in a separate license agreement or EULA. The terms should explicitly cover permitted installation and use, redistribution, modification, warranty disclaimers, liability, and termination. This is the closest fit when the owner does not want to grant general rights to modify or redistribute the software. Obtain legal review before distribution.

### Custom Source-Available Terms

A custom agreement can allow selected uses while restricting others, such as redistribution or commercial use. These restrictions require careful drafting and may mean the project is not considered open source under common open-source definitions. Obtain legal review and state the restrictions clearly.

## Common Open-Source Alternatives

These licenses preserve the owner's copyright but grant recipients broader rights than a proprietary license:

- **MIT or BSD-3-Clause:** Short, permissive terms that allow use, modification, and redistribution, subject to attribution and notice requirements. They provide little control over downstream reuse.
- **Apache License 2.0:** Permissive reuse with explicit patent-related terms and notice requirements. It still allows redistribution and commercial use.
- **GNU GPLv3:** Allows use, modification, and redistribution subject to copyleft conditions, including source-code obligations for distributed derivative works. It does not preserve exclusive control over downstream redistribution.

## Decision Before Release

The owner should decide whether FocusForge will be proprietary, source-available under custom terms, or open source; identify the copyright holder; and have the selected terms reviewed. Only after approval should a corresponding `LICENSE` or EULA be added and the CPack installer configured to display those actual terms. No license has been chosen or applied as part of this documentation change.
