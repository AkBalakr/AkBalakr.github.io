const DEFAULT_OPTIONS = {
    startOffsetY: 24,
    bottomPadding: 30,
    platformGapMin: 66,
    platformGapMax: 138,
    platformMinWidth: 60,
    platformMaxWidth: 128,
    platformInset: 8,
    jumpHeightMin: 18,
    jumpHeightMax: 46,
    jumpUpMinMs: 160,
    jumpUpMaxMs: 260,
    jumpDownMinMs: 240,
    jumpDownMaxMs: 390,
    jumpDelayMinMs: 340,
    jumpDelayMaxMs: 920
};

function randomBetween(min, max) {
    return min + Math.random() * (max - min);
}

function documentHeight() {
    return Math.max(
        document.body.scrollHeight,
        document.body.offsetHeight,
        document.documentElement.scrollHeight,
        document.documentElement.offsetHeight,
        window.innerHeight
    );
}

export class LeftRailPlatforms {
    constructor(options = {}) {
        this.options = { ...DEFAULT_OPTIONS, ...options };

        this.root = null;
        this.track = null;
        this.characterAnchor = null;
        this.character = null;
        this.shadow = null;

        this.resizeDebounceTimer = null;
        this.jumpTimer = null;
        this.isDestroyed = false;

        this.handleResize = this.handleResize.bind(this);
    }

    mount() {
        if (this.root || this.isDestroyed) {
            return;
        }

        this.createDOM();
        this.regeneratePlatforms();

        window.addEventListener("resize", this.handleResize, { passive: true });
        window.addEventListener("orientationchange", this.handleResize, { passive: true });

        this.startJumpLoop();
    }

    createDOM() {
        this.root = document.createElement("div");
        this.root.className = "ambient-left-layer";

        this.track = document.createElement("div");
        this.track.className = "ambient-platform-track";
        this.root.append(this.track);

        this.characterAnchor = document.createElement("div");
        this.characterAnchor.className = "ambient-character-anchor";
        this.characterAnchor.innerHTML = `
            <div class="ambient-character-shadow"></div>
            <div class="ambient-character" aria-hidden="true">
                <div class="ambient-character-body"></div>
                <div class="ambient-character-head">
                    <span class="ambient-character-eye left"></span>
                    <span class="ambient-character-eye right"></span>
                </div>
            </div>
        `;

        document.body.append(this.root, this.characterAnchor);

        this.character = this.characterAnchor.querySelector(".ambient-character");
        this.shadow = this.characterAnchor.querySelector(".ambient-character-shadow");
    }

    regeneratePlatforms() {
        if (!this.root || !this.track) {
            return;
        }

        const fullHeight = documentHeight();
        this.root.style.height = `${fullHeight}px`;
        this.track.innerHTML = "";

        const laneWidth = this.root.getBoundingClientRect().width || 160;
        const safeWidth = Math.max(52, laneWidth - this.options.platformInset * 2);

        let y = this.options.startOffsetY;

        while (y < fullHeight - this.options.bottomPadding) {
            const width = randomBetween(this.options.platformMinWidth, this.options.platformMaxWidth);
            const clampedWidth = Math.min(width, safeWidth);

            const maxX = Math.max(
                this.options.platformInset,
                laneWidth - clampedWidth - this.options.platformInset
            );
            const x = randomBetween(this.options.platformInset, maxX);

            const platform = document.createElement("div");
            platform.className = "ambient-platform";
            platform.style.left = `${Math.round(x)}px`;
            platform.style.top = `${Math.round(y)}px`;
            platform.style.width = `${Math.round(clampedWidth)}px`;
            platform.style.opacity = randomBetween(0.58, 0.94).toFixed(2);
            platform.style.transform = `skewX(${randomBetween(-20, -12).toFixed(1)}deg)`;

            this.track.append(platform);

            y += randomBetween(this.options.platformGapMin, this.options.platformGapMax);
        }
    }

    startJumpLoop() {
        if (!this.character || !this.shadow) {
            return;
        }

        if (window.matchMedia("(prefers-reduced-motion: reduce)").matches) {
            return;
        }

        const jumpOnce = () => {
            if (this.isDestroyed || !this.character || !this.shadow) {
                return;
            }

            const lift = randomBetween(this.options.jumpHeightMin, this.options.jumpHeightMax);
            const upMs = randomBetween(this.options.jumpUpMinMs, this.options.jumpUpMaxMs);
            const downMs = randomBetween(this.options.jumpDownMinMs, this.options.jumpDownMaxMs);
            const duration = Math.round(upMs + downMs);

            this.character.animate(
                [
                    { transform: "translateY(0)" },
                    { transform: `translateY(${-lift}px)`, offset: upMs / duration, easing: "cubic-bezier(0.22, 1, 0.36, 1)" },
                    { transform: "translateY(0)" }
                ],
                {
                    duration,
                    easing: "cubic-bezier(0.12, 0.8, 0.2, 1)",
                    fill: "none"
                }
            );

            this.shadow.animate(
                [
                    { transform: "scale(1)", opacity: 0.32 },
                    { transform: "scale(0.72)", opacity: 0.2, offset: upMs / duration },
                    { transform: "scale(1)", opacity: 0.32 }
                ],
                {
                    duration,
                    easing: "ease-out",
                    fill: "none"
                }
            );

            const pauseMs = randomBetween(this.options.jumpDelayMinMs, this.options.jumpDelayMaxMs);
            this.jumpTimer = window.setTimeout(jumpOnce, duration + pauseMs);
        };

        this.jumpTimer = window.setTimeout(jumpOnce, randomBetween(250, 900));
    }

    handleResize() {
        if (this.isDestroyed) {
            return;
        }

        if (this.resizeDebounceTimer) {
            clearTimeout(this.resizeDebounceTimer);
        }

        this.resizeDebounceTimer = window.setTimeout(() => {
            this.regeneratePlatforms();
        }, 120);
    }

    destroy() {
        this.isDestroyed = true;

        window.removeEventListener("resize", this.handleResize);
        window.removeEventListener("orientationchange", this.handleResize);

        if (this.resizeDebounceTimer) {
            clearTimeout(this.resizeDebounceTimer);
        }

        if (this.jumpTimer) {
            clearTimeout(this.jumpTimer);
        }

        if (this.root) {
            this.root.remove();
        }

        if (this.characterAnchor) {
            this.characterAnchor.remove();
        }

        this.root = null;
        this.track = null;
        this.characterAnchor = null;
        this.character = null;
        this.shadow = null;
    }
}

export function initLeftRailPlatforms(options = {}) {
    const module = new LeftRailPlatforms(options);
    module.mount();
    return module;
}
