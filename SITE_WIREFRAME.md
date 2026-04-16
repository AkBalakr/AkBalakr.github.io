# Full Site Wireframe (Pre-Selection)

Purpose: define the complete structure of the portfolio before choosing final art style, imagery, or micro-layout details.

## 1) Site Map

- Home: index.html
- About: about.html
- Projects List: projects.html
- Shared Header: components/header.html (used on About and Projects List)
- Embedded Project Demos inside Home:
  - Flappy Bird AI: projects/flappy-bird/flappy.html
  - Chess AI: projects/chess/chess.html
  - RSA Cryptography: projects/rsa/rsa.html

## 2) Global Navigation Flow

Desktop and mobile flow:

[Header/Nav]
  -> Home
  -> Projects List
  -> About
  -> Resume (external file)
  -> LinkedIn (external)

Home-only sticky nav flow:

[About anchor at top]
  -> [Projects anchor lower on page]

## 3) Home Page Wireframe (index.html)

### 3.1 Desktop (>= 901px)

+----------------------------------------------------------------------------------+
| Sticky Nav: logo | About | Projects                                              |
+----------------------------------------------------------------------------------+
|                              HERO / PROFILE SECTION                              |
| +--------------------------------------------------+  +------------------------+ |
| | Left Column                                       |  | Right Column           | |
| | - Availability eyebrow                            |  | Experience Card        | |
| | - Name / title                                    |  | - Timeline items       | |
| | - Bio                                              | |                        | |
| | - Contact links                                    | | Character slot overlaps| |
| | - ART SLOT A: 520 x 220                            | | top edge of card       | |
| +--------------------------------------------------+  +------------------------+ |
+----------------------------------------------------------------------------------+
| Section divider: // Projects ----------------------------------------------------|
+----------------------------------------------------------------------------------+
| Tab nav: // Projects                                                             |
+----------------------------------------------------------------------------------+
| ART SLOT C: 1080 x 160                                                           |
+----------------------------------------------------------------------------------+
| Project 001 label + Flappy iframe card                                           |
+----------------------------------------------------------------------------------+
| ART SLOT D: 1080 x 120                                                           |
+----------------------------------------------------------------------------------+
| Project 002 label + Chess iframe card                                            |
+----------------------------------------------------------------------------------+
| ART SLOT E: 1080 x 120                                                           |
+----------------------------------------------------------------------------------+
| Project 003 label + RSA iframe card                                              |
+----------------------------------------------------------------------------------+

### 3.2 Mobile (<= 900px)

+------------------------------------------------------+
| Sticky Nav (compressed)                              |
+------------------------------------------------------+
| Hero single-column stack                             |
| - Name / title / bio / links                         |
| - ART SLOT A resized to 100% x 170                   |
| - Experience card                                    |
| - Character slot B resized to 240 x 105              |
+------------------------------------------------------+
| // Projects divider + tab                            |
+------------------------------------------------------+
| ART SLOT C resized to 100% x 130                     |
+------------------------------------------------------+
| Flappy iframe                                        |
+------------------------------------------------------+
| ART SLOT D resized to 100% x 95                      |
+------------------------------------------------------+
| Chess iframe                                         |
+------------------------------------------------------+
| ART SLOT E resized to 100% x 95                      |
+------------------------------------------------------+
| RSA iframe                                           |
+------------------------------------------------------+

## 4) About Page Wireframe (about.html)

### Desktop

+----------------------------------------------------------------------------------+
| Shared Header (component)                                                        |
+----------------------------------------------------------------------------------+
| Hero card                                                                         |
| +--------------------------------------+  +------------------------------------+ |
| | Text block                             | | Portrait image                     | |
| | - About title                          | | 400 x 400 display target           | |
| | - 2 short paragraphs                   | |                                    | |
| +--------------------------------------+  +------------------------------------+ |
+----------------------------------------------------------------------------------+

### Mobile

+------------------------------------------------------+
| Shared Header with hamburger                          |
+------------------------------------------------------+
| About hero card                                      |
| - Title                                              |
| - Paragraphs                                         |
| - Portrait image centered below text                 |
+------------------------------------------------------+

## 5) Projects List Page Wireframe (projects.html)

### Desktop

+----------------------------------------------------------------------------------+
| Shared Header                                                                    |
+----------------------------------------------------------------------------------+
| Hero container                                                                   |
| - Heading: My Projects                                                           |
| - Card grid (2-column at medium/large)                                           |
|   - Project card 1                                                               |
|   - Project card 2                                                               |
|   - Project card 3                                                               |
+----------------------------------------------------------------------------------+

### Mobile

+------------------------------------------------------+
| Shared Header with hamburger                          |
+------------------------------------------------------+
| Projects hero                                         |
| - Heading                                             |
| - Project cards in single column                      |
+------------------------------------------------------+

## 6) Embedded Project Demo Wireframe (Common Pattern)

All three embedded projects use this macro structure:

+----------------------------------------------------------------------------------+
| project-card (2 columns)                                                         |
| +-----------------------------------------------+  +---------------------------+ |
| | Left: Demo Area                               |  | Right: Info Panel         | |
| | - Demo header (window dots + file label)      |  | - Project number/name     | |
| | - Project-specific interactive content         |  | - Description             | |
| | - Controls bar                                 |  | - Parameters/optimizations| |
| | - Stats bar                                    |  | - Skills tags             | |
| +-----------------------------------------------+  +---------------------------+ |
+----------------------------------------------------------------------------------+

Per project specifics:

- Flappy:
  - Canvas playfield in fixed wrapper
  - Start/reset/population/speed controls
  - Neural input readouts

- Chess:
  - Mode toggle (Human vs Bot / Bot vs Bot)
  - Inner tabs (Board / Weights)
  - Board + captures + status + win tracker

- RSA:
  - Inner tabs (Encrypt/Decrypt, Math, Keys)
  - Crypto textareas + action buttons
  - Math explainer blocks + key tables

## 7) Art Placement Decision Layer (Before Final Choice)

Current reserved slots on Home:

- Slot A (Hero art): 520 x 220 desktop, full width x 170 mobile
- Slot B (Character overlap on Experience): 300 x 130 desktop, 240 x 105 mobile
- Slot C (Projects banner): 1080 x 160 desktop, full width x 130 mobile
- Slot D (Project divider): 1080 x 120 desktop, full width x 95 mobile
- Slot E (Project divider): 1080 x 120 desktop, full width x 95 mobile

Recommendation before final art insertion:

1. Lock one global visual style for all slots (line-art, painterly, or pixel style).
2. Decide whether slots C/D/E should be one continuous scene split into bands or separate pieces.
3. Confirm character pose for slot B with transparent background and downward shadow overlap.

## 8) Implementation Sequence (after you choose style)

1. Replace Slot B first (highest visual impact).
2. Replace Slot A second (hero identity anchor).
3. Replace Slot C then D/E for narrative continuity through projects.
4. Tune overlap offsets and responsive crop once real assets are in place.
