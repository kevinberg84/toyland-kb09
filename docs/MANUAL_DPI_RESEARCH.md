# KB-09 manual research: native HiDPI rendering in Windows hosts

Research checked **2026-10-07**. This is a source-backed planning note, not the finished manual or a DAW compatibility certification. Scope is the current Windows VST3 instrument. No additional DAWs were installed, opened or tested for this research. The user confirmed that disabling Live's plug-in auto-scaling fixed the display; the latest report did not confirm a fresh-instance step or identify the precise Live build. Other entries distinguish vendor instructions from proposed KB-09 troubleshooting.

Native HiDPI rendering and window size are different concerns. A host can enlarge a low-resolution bitmap to a comfortable size while making its lettering blurry. Prefer allowing KB-09 to render at the display's scale, then use its own M/L controls for size. That is our product recommendation, not a claim that every host exposes the same switch. Native labels can render sharply while the photographic surfaces retain their finite source detail; neither a large window nor a 3840-pixel export proves a physical 4K-display test.

## Evidence overview

| Host documentation checked | Setting scope | Proposed KB-09 approach | Evidence boundary |
|---|---|---|---|
| Ableton Live Windows support article; project uses Live 11 Suite | Plug-in context-menu choice; separate application-wide HiDPI preference | Disable plug-in auto-scaling; test a fresh instance only as a follow-up check | User confirmed toggle success; fresh-instance action and precise build unreported; vendor article has a VST3 caveat |
| Cubase Pro 15.0; Nuendo 14.0 English help | Plug-in-specific compatibility option; separate global preference | Keep application HiDPI enabled; avoid legacy plug-in resizing assistance | Documented controls; KB-09 host result untested |
| Studio One 4.1+ technical note; current Fender Studio Pro 8.1 manual | Per-plug-in bitmap-scaling option; separate global preference | Keep global HiDPI; turn off the plug-in bitmap-scaling option if offered | Current official manual corroborates behaviour; Studio One 7 exact UI needs confirmation |
| FL Studio current rolling online manual | Wrapper settings and GUI presets; separate whole-DAW scaling | Permit the plug-in's own DPI handling; use bridging only as a targeted troubleshooting step | Vendor describes both paths; KB-09 host result untested |
| REAPER 7.81 guide; Windows feature introduced in 6.02 | Individual plug-in default, affecting floating windows | Leave compatibility scaling off for native rendering | Menu/persistence documented; KB-09 recommendation needs host test |
| Bitwig current public guide, 2.5 release notes, maintainer demonstration in 3.3.7 | Global plug-in preference plus per-plug-in stretch option on Windows | Disable stretching for KB-09 if the documented option is present | Exact current-version UI and reload behaviour remain unverified |

## Ableton Live 11 context: confirmed toggle result and proposed follow-up

**Human report, 2026-10-07:** the user confirmed that disabling **Auto-Scale Plug-in Window** fixed the display. The project uses Live 11 Suite, but the latest report did not supply its precise build, display resolution/scaling, mixed-DPI measurements or a fresh-instance action. Do not attribute those details to the user. Record the actual environment and KB-09 module hash when obtaining further test evidence.

Proposed manual instruction: find KB-09 in Live's Browser, open its context menu and clear the setting above. If further verification is needed, close/reopen the editor and test a newly inserted instance; this is our follow-up advice, not a user-confirmed action or established requirement. Preserve an existing sound with KB-09 Save before replacing an instance. This is a plug-in setting, not a reason to disable Live's overall HiDPI support.

A JUCE maintainer independently describes native-resolution VST3 rendering in Live 11 with auto-scaling disabled and notes that removing/re-adding the plug-in may be necessary. That supports the conditional troubleshooting suggestion; it does not add an unreported action to the user's confirmation. [JUCE maintainer explanation, May 2024](https://forum.juce.com/t/blurry-vst3-in-ableton-live-11-hidpi/61587).

**Source discrepancy:** Ableton documents the context menu on both the plug-in title bar and Browser entry, but the current article explicitly excludes VST3 from that feature and says VST3 is automatically scaled. Cite the successful toggle result as the user's report, not as a universal VST3 promise from Ableton. The article does not specify a fresh-instance or restart requirement. [Ableton: High-DPI monitor support](https://help.ableton.com/hc/en-us/articles/209775985-High-DPI-monitor-support).

## Cubase and Nuendo

On Windows, keep **Preferences > General > Enable HiDPI** enabled for native application rendering. Cubase 15 documents standard display factors from 100% through 200% in 25% steps; it does not promise arbitrary factors such as 133%. [Cubase Pro 15: General](https://www.steinberg.help/r/cubase-pro/15.0/en/cubase_nuendo/topics/preferences/preferences_general_r.html).

Open KB-09's host control panel and its **Functions** menu, or right-click unused panel space. Find **Allow Window to be Resized**. Steinberg describes this as plug-in-specific assistance for third-party plug-ins without their own DPI handling. Our proposed native-rendering baseline is to leave it unchecked for KB-09. Cubase 15 also lists **User Interface Scaling** in this menu; that is a separate size control, and this research does not establish how it interacts with KB-09's M/L sizing. [Cubase Pro 15: effect control-panel menu](https://www.steinberg.help/r/cubase-pro/15.0/en/cubase_nuendo/topics/audio_effects/audio_effects_control_panel_context_menu_r.html), [Nuendo 14: corresponding menu](https://www.steinberg.help/r/nuendo/14.0/en/cubase_nuendo/topics/audio_effects/audio_effects_control_panel_context_menu_r.html).

The cited menu pages do not state a DAW restart requirement or define whether every existing instance updates immediately. For validation, close/reopen the editor and test a fresh instance; treat this as our test procedure. Nuendo 15's English page could not be retrieved in this pass, so the English Nuendo instruction is explicitly versioned to 14. These documentation entries are titled for effects; confirm the same host header controls on the actual KB-09 instrument window before publishing screenshots.

## Studio One / current Fender Studio Pro

The old PreSonus online-manual URL now redirects to Fender's manual. The current page identifies itself as **Fender Studio Pro 8.1**. It advises retaining the Windows application setting **Studio Pro > Options > General > Enable High-DPI Mode**. For an individual plug-in, open the settings menu on its editor tab and turn off **Enable System DPI Scaling** when that plug-in handles scaling itself. The manual describes that option as a remedy for older, small interfaces, warns of blur and advises avoiding it with self-scaling plug-ins. [Current official General Options](https://fenderstudiopromanual.fender.com/en/Content/Setup_Topics/General_Options.htm).

PreSonus introduced the per-plug-in option in **Studio One 4.1**, requiring **Windows 10 version 1803 or later**. Its developer note confirms that this places the plug-in in a DPI-unaware context so Windows bitmap-scales the interface. This is not equivalent to native high-resolution drawing. [PreSonus technical note, August 2018](https://presonus.software/developer/technical-note-system-dpi-scaling.pdf).

For Studio One 7, consult **Help > Studio One Reference Manual** and search the setting name before finalizing the exact menu screenshot. Do not rename its application menu to Studio Pro in instructions for an older installation. [PreSonus: opening/searching the Studio One 7 manual](https://support.presonus.com/hc/en-us/articles/8773820563981--8b-Studio-One-7-Using-the-Manual-to-Search-for-Help-Topics).

The current scaling page specifies neither a restart requirement for the per-plug-in toggle nor its persistence across all instances. Reopen the editor and test a fresh instance as a verification step. If the option is absent, do not claim it must be present for every VST3 or advise disabling the entire DAW's HiDPI mode as a substitute.

## FL Studio

Windows instructions from the current rolling manual:

1. Open the plug-in wrapper's additional settings, then its **Settings** tab. Enable **DPI aware** so the plug-in handles its own scaling.
2. If a scaling problem remains, Image-Line's documented native-rescaling route is **Processing > Make bridged** on and **DPI aware when bridged** on. The manual uses both DPI labels; match the actual installed wrapper. This is a troubleshooting path, not evidence that KB-09 requires bridging.
3. Save a **favorite preset** after configuring GUI behaviour. Image-Line warns that GUI/scaling settings are stored with presets and are not all covered by wrapper option autosave.

These are wrapper/plug-in settings. The opposite recipe—external window plus DPI awareness off—is for plug-ins without native rescaling. Bridging changes process hosting and can affect audio-device focus, so do not prescribe it automatically to every KB-09 user. The wrapper page gives no universal restart requirement for these changes. Test a new instance loaded from the saved favorite. [Image-Line: Plugin Wrapper](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/plugins/wrapper.htm).

Whole-DAW scaling is separate: **Options > General settings** (F10), under Display. Image-Line explicitly requires an FL Studio restart after changing **Main GUI Scaling**. Changing that setting is not the first proposed remedy for one blurry native-DPI plug-in. [Image-Line: General settings](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/envsettings_general.htm).

The online manual does not identify a fixed release number for these pages; screenshot the installed FL Studio version before publishing a version-specific walkthrough. The Windows bridging instructions should not be copied into macOS instructions.

## REAPER

In **Add FX**, locate KB-09, right-click its name, then open **Compatibility settings > HiDPI compatibility mode when floating windows**. Our proposed native-rendering baseline is unchecked. The 7.81 guide documents this under section 6.10, individual plug-in defaults: the selection applies when a new instance is added. It also documents the plug-in-window **+** menu beside Param for compatibility settings in section 6.19. [REAPER 7.81 official guide](https://www.reaper.fm/userguide/ReaperUserGuide781c.pdf), [official HTML edition](https://dlx.reaper.fm/userguide/html/).

This is a per-plug-in setting for floating windows; it is not a general zoom control for the docked FX chain or all REAPER windows. Test a newly added instance in a floating window after changing the browser default. The guide does not state a DAW restart requirement for this choice. Its label and menu are documented; unchecked as the preferred KB-09 baseline remains a recommendation to validate. Cockos introduced the feature specifically for Windows in version 6.02. [Official REAPER 6.x release notes](https://www.reaper.fm/download-old.php?ver=6x).

The download page offered REAPER 7.82 when researched, while its current guide was 7.81. Do not imply a local 7.82 test occurred. [Official guide index](https://www.reaper.fm/userguide.php).

## Bitwig Studio

Bitwig's own 2.5 release notes document a **Windows per-plug-in context-menu choice for automatic interface stretching**. [Official 2.5 release notes](https://downloads.bitwig.com/stable/2.5.1/Release-Notes-2.5.1.html).

For exact labels, Sonic Charge maintainers demonstrated Bitwig **3.3.7 on Windows**: retain **Plug-in Interface Scaling**, then right-click the plug-in in the Browser and disable **Stretch Plug-in Window to Match DPI** for native detail. Their post includes side-by-side rendering evidence. This is first-hand plug-in-maintainer guidance, not a current Bitwig 6.x support statement or a KB-09 test. [Sonic Charge maintainers' HiDPI discussion](https://soniccharge.com/forum/topic/1856-hidpi-display).

The documented global location for plug-in preferences is **Dashboard > Settings > Plug-ins**. Bitwig's current public plug-in chapter does not explain the DPI toggle, and its main UI scaling is a separate Dashboard preference. Thus a conditional KB-09 troubleshooting instruction may use the labels above **if present**, but the final manual should verify the current Windows UI before asserting exact behaviour. [Bitwig: plug-in handling](https://www.bitwig.com/userguide/latest/vst_plug-in_handling_and_options/), [Bitwig: Dashboard](https://www.bitwig.com/userguide/latest/the_dashboard/).

The primary sources reviewed do not establish a DPI-toggle restart/reload requirement. Reopen the editor and test a fresh instance as our verification procedure. The current guide's explicit reload rule concerns **hosting mode**, so it must not be misquoted as a rule for DPI scaling. No equivalent toggle for macOS or Linux was established here; do not invent one. Bitwig's support page says its user guide is being revised for 6/6.1, which reinforces the need for a current UI check. [Bitwig support](https://www.bitwig.com/support/?change_language=en).

## Before turning this research into user instructions

For each Windows host/build proposed for the manual, verify normal loading first, then record the exact toggle state, display resolution/scaling, editor reopen behaviour, fresh-instance behaviour and preset retention. Check native text/values, mouse targeting, full header/footer, M/L sizing and movement between differently scaled monitors when such hardware is available. This is a proposed validation list, not completed testing.

Keep each DAW recipe separate. Do not equate host compatibility enlargement, operating-system scaling, plug-in zoom and KB-09's M/L window choices. Do not prescribe lowering monitor resolution or disabling global HiDPI as the standard way to obtain sharper text. When a control is absent or its current behaviour is unverified, state that boundary and use the tested baseline rather than inventing a universal switch.

Pro Tools and Logic instructions belong to future tested platform/format work. This Windows VST3 research should not imply an AAX/AU release or validated support in those hosts.

## Manual planning: give Condition / Age its own prominent section

Place **Condition / Age — from fresh to worn** near the initial sound and interface walkthrough, with a clear contents entry. Present it as an optional creative control: users may keep a fresh appearance or explore increasing wear, and return to their previous setting. Plan around the user's latest requested treatment: stronger gradual yellowing of the ivory keys, gradual case patina, separate individual stains that keep fixed geometry after appearing, and **only one cracked key**. Describe these as implemented only after checking the revised source and actual compiled-editor pictures. Cosmetic appearance must remain distinct from control functionality.

Knob and fader visual work is deferred until the user supplies personal reference photos. Do not present proposed knob/cap revisions as finished or expand that work while drafting this research note.

Describe its sonic effects separately from appearance. Current engine source uses Age in pitch offsets, filter behaviour and envelope timing; confirm the final formulas and audible examples before drafting those paragraphs. Do not describe the feature as a measured decades-of-aging simulator, a verified replica of deteriorated original components or proof of exact circuit/hardware fidelity. Reference `Source/DSP/Engine.cpp` and `docs/GUI_MATERIAL_AGING.md` when writing that section. This note schedules the section; it does not draft the full manual or create a download package.
