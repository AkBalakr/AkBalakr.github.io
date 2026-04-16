# Platforming Character Requirements

## Purpose
This document captures all requested requirements for the homepage left-side platform and character feature, including requirement changes over time.

## Feature Scope
- Homepage ambient left-side visual module.
- Procedurally generated platforms.
- Small character animation behavior.
- Module-based implementation so behavior can be enabled or disabled from homepage markup.

## Source and Integration Points
- Module file: assets/js/left-rail-platforms.js
- Homepage integration: index.html
- Homepage styling: index.css

## Requirement History (All Requested)

### R1. Left-Side Platform Lane
- Add platforms on the left side of the homepage.
- Platforms should appear vertically distributed (stacked down the page/track).
- Platform positions should be randomly generated.

### R2. Character Presence
- Add a small animated character near the left-side platform lane.
- Character should appear around middle screen height as part of the scene.

### R3. Modular Implementation
- Build this as a module, not inline-only logic.
- Module should expose an initializer that can be called from homepage script.

### R4. Scroll-Linked Retargeting Request (Earlier)
- Character should move with scroll context.
- Character should set a destination platform near the middle height of the viewport.
- Character should jump toward that destination platform.

### R5. Mouse-Driven Platform Motion Request (Latest Behavior Request)
- Platform movement should be controlled only by mouse movement up/down.
- Platform lane should not retarget based on page scroll.

### R6. Independent Character Tempo
- Character should jump at its own pace.
- Character jumping should be independent of platform retargeting logic.

### R7. Temporary Disable Request (Current State)
- Side character movement should be commented out for now.
- Keep code in place, but disable initialization.

## Requirement Priority and Conflict Notes
- R5 and R6 are the latest behavior direction and should be treated as active movement requirements.
- R4 conflicts with R5 and is retained only as prior requested behavior for reference.
- R7 is the current runtime state and takes effect until re-enabled.

## Current Status Snapshot
- Module initialization is currently commented out in index.html.
- Feature code remains in assets/js/left-rail-platforms.js for future revision.
- Styling hooks remain in index.css.

## Acceptance Criteria for Next Implementation Pass
1. Feature remains disabled until explicitly re-enabled.
2. When re-enabled, platform lane vertical offset responds only to mouse Y movement.
3. Character jump loop runs on its own timing cadence.
4. No scroll-based destination targeting is active.
5. Module can be switched on/off from homepage integration block.
