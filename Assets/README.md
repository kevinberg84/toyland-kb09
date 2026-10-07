# Toyland KB-09 panel assets

The 0.11.0 editor combines generated photographic materials with native JUCE controls, text, scales, switches and indicators. The supplied corrected marks retain KEVINBERG 09 / KB-09 on the left and Toyland on the right. Research photographs informed proportions; they are not repackaged as runtime textures. No Universal Audio artwork is used.

The current CMake target embeds nine PNG files. [Photoreal/README.md](Photoreal/README.md) is the authoritative inventory of actual dimensions and runtime crops; [Photoreal/PROMPTS.md](Photoreal/PROMPTS.md) records saved production prompts and built-in imagegen provenance. Historical generated images remain for provenance and are not all compiled into the plugin.

The 37-decal wear atlas assigns a separate stain to each of 32 keys, two independent key cracks and three independent rotary corrosion patches. Six other case marks are each used once on the case. Stains appear at fixed size and opacity; yellowing/rust develop gradually. All fader caps remain fitted and their solid bodies have opaque registered backing beneath the photographic surface/shadow.

Artwork is loaded from compiled memory. No external texture, font file, sample library or network service is needed at runtime. Arial uses the system font API with fallback. `Branding.h` defines the single product identity; historical branding headers and the inert legacy parameter remain for compatibility only.

Native text and geometry render at the output scale, with high-quality sampling for photographic materials. The actual editor exports 3840x2160 and 2880x1620 previews. Finite rasters remain finite: the chassis is 1774x887, so its smooth shading is enlarged at 4K rather than becoming a native 4K photograph. [HIDPI guidance](../docs/HIDPI.md) separates host bitmap scaling from window size and source detail. [Validation](../docs/VALIDATION.md) records 280 passing GUI/asset checks and the exact installed module.
