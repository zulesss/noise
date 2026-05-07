# HANDOFF — ЦВЕТ-ШУМ Phase 1 D3

**Updated:** 2026-05-04
**Last commit on `main`:** `8a2c28e0` (phase1 d3: LVL_Sluz whitebox spec)
**Active phase:** Phase 1 Day 3 (in progress)

Этот документ — entry point для новой Claude Code сессии. Прочитать **в первую очередь**, потом ссылки внутри.

---

## 1. Что за проект

**ЦВЕТ-ШУМ** (codename `noise`) — first-person atmospheric horror-puzzle на UE5.7. Советская АЭС, 26 апреля 1986, ночная смена. 25-35 минут одно прохождение, 2 концовки, 14-day prototype, $0 budget. Полная identity / core fantasy / mechanic specs в [PLAN.md](PLAN.md).

---

## 2. Setup и окружение

| Где | Что |
|---|---|
| Linux dev box (Claude Code) | `/home/azukki/cvet-shum` — авторинг текста, design docs, C++ source через `unreal-engineer` агента, git CLI |
| Windows machine | `C:\Unreal Projects\noise` — UE5.7 Editor, VS 2022 Community для C++ compile, manual playtest |
| GitHub remote | `git@github.com:zulesss/noise.git` (или HTTPS) |

**Workflow:** Linux пишет text/code/config/git → push → Windows pull → UE Editor build/play → push back.

UE5 binary **только** на Windows. Linux dev box без UnrealEditor — headless validation (`CompileAllBlueprints`) **недоступна**. Все compile errors разрешаются на Windows-стороне с copy-paste error log'а в чат.

См. [WINDOWS_SETUP.md](WINDOWS_SETUP.md) — полный one-time setup checklist.

---

## 3. Resolved decisions / ADRs (хронологически)

### 3.1 Design (5 вопросов от design-агентов)
- **Q1 финальный пульт:** **без фиксации** чисел в журнале. Paranoia route — игрок сомневается, видел ли он 14 или 11 зелёных. (PLAN.md, .claude/context.md)
- **Q2 дверь Секция В → Control Room:** **always open**. Никаких trigger-conditions кроме финального lock'а после 4.2.
- **Q3 walk speed:** **240 cm/s lock на Day 1**. Recalibrate D5-6 — если тягомотина, до 300 (не 480). Shift = 320 cm/s.
- **Q4 65-сек telephone UX cue:** **прямая инструкция в пункте 3.2 регламента**: "не вешать трубку до ответа диспетчера". Никаких UI-prompt'ов.
- **Q5 CLOCK_WALKS_BACK:** **оставить тихой**. Опциональная anomaly без процедурного крючка. Часть аномалий должна оставаться на periphery.

### 3.2 Tech / asset
- **GASP не используем** — third-person motion matching showcase, 0% полезной поверхности для slow-walk first-person horror без traversal/jump/crouch. Replaced by стоковый UE5 `Character` + `ANoiseCharacter` C++ base.
- **Soviet/Post Soviet pack отвалился** из Fab Library. Replaced by Cold War Bunker (для дверей/модулей) + Sketchfab CC-BY hunt (locker, operator desk, штучные props) + Material decals (PolyHaven CC0, AmbientCG — peeling paint, warning signs).
- **Packs at root `Content/<OriginalName>/`** — не двигаем через Move. UE5 хранит hardcoded `/Game/<original>/...` references внутри `.uasset`. Каждый Move риск broken refs. Наша authored работа — в организованных папках (`Audio/`, `Blueprints/`, `Levels/`, etc.).
- **C++ project, не pure BP.** C++ base classes (`ANoiseCharacter`, `ANoiseGameModeBase`) + тонкий BP child для designer-friendly tweaks. Альтернатива — ассемблить 200+ BP nodes вручную из text-спеки — отвергнута.
- **Project lives in `C:\Unreal Projects\noise`**, НЕ OneDrive. OneDrive locks `.uasset` во время sync'а + наезжает на Windows env var limit при build'е.
- **VS 2022 (17.x), не VS 2026 (18.x).** UE5.7 официально поддерживает только 2022. VS 2026 ломал VisualStudioTools plugin RulesError + Windows env-too-long.
- **`VisualStudioTools` plugin disabled** в `noise.uproject` — broken plugin в user's UE5.7 install (RulesError при парсинге module rules). Не включать обратно даже если wizard re-enables.
- **Flat C++ структура** — все классы в `Source/noise/` root, не в Player/ + Game/ subfolders. UE5.7 UBT не резолвит `#include "Player/X.h"` из subfolders без явного `PrivateIncludePaths` config'а в Build.cs. Когда будет 10+ классов — рассмотрим Public/Private split.
- **MCP для UE — deferred** до Phase 2+. Все варианты alpha-grade, плюс Linux→Windows network proxy сложен. Текущий text-spec workflow optimal для Phase 1.

---

## 4. Что уже сделано (Phase 1 D1-D2)

### Code / config
- `noise.uproject` — UE5.7, Modules.noise (Runtime), Plugins (Niagara, EnhancedInput, ModelingToolsEditorMode, **VisualStudioTools=false**)
- `Source/noise.Target.cs`, `noiseEditor.Target.cs` — `BuildSettingsVersion.V6`, `IncludeOrderVersion.Latest` (или Unreal5_7)
- `Source/noise/noise.{Build.cs, h, cpp}` — wizard's IMPLEMENT_PRIMARY_GAME_MODULE
- `Source/noise/NoiseCharacter.{h,cpp}` — first-person base, FOV 75, walk 240/shift 320, head bob math, pitch +75°/-60°, mouse 0.18 BP variable. Reference: [docs/feel/atmosphere_plan.md §1](docs/feel/atmosphere_plan.md). Spec: [docs/dev/bp_player_spec.md](docs/dev/bp_player_spec.md).
- `Source/noise/NoiseGameModeBase.{h,cpp}` — DefaultPawnClass = ANoiseCharacter::StaticClass()
- `Config/DefaultEngine.ini` — Lumen GI/Reflections, Volumetric Fog, Bloom Convolution, Motion Blur OFF, Audio 48 kHz, GameMapsSettings (LVL_TestWalk, BP_GameMode_Default)
- `Config/DefaultInput.ini` — WASD/Mouse/E/Shift/Tab legacy mappings, LookYaw/LookPitch scale=1.0 (sensitivity в C++ var)
- `Config/DefaultGame.ini` — ProjectName, ProjectDisplayedTitle "ЦВЕТ-ШУМ" через NSLOCTEXT
- `.gitignore` — UE5 standard + `*.sln`, `Backup/`, `UpgradeLog.htm`, `shadertoolsconfig.json`
- `.gitattributes` — LFS для `*.uasset`, `*.umap`, `*.fbx`, `*.wav`, `*.png`, `*.tga`

### Assets / Editor side (Windows-pushed)
- `Content/Blueprints/Player/BP_Player.uasset` — extends ANoiseCharacter (Blueprint child)
- `Content/Blueprints/BP_GameMode_Default.uasset` — extends ANoiseGameModeBase, DefaultPawnClass = BP_Player
- `Content/Levels/LVL_Empty.umap` — пустой baseline level
- `Content/Levels/LVL_TestWalk.umap` — basic level с floor + sky + light для feel-test'а
- `Content/ColdWarBunker/` — 152 mesh, paid pack
- `Content/Megascans/` (или `Content/DerelictCorridor/`) — 262 industrial assets
- `Content/NiagaraExamples/` — VFX samples (50+ systems)
- `Content/__ExternalActors__/`, `Content/__ExternalObjects__/` — UE5 World Partition system folders
- Empty placeholder folders с `.gitkeep`: `Content/{Audio, Blueprints, Data, Levels, Materials, PostProcess, UI, VFX, Sketchfab}/`

### Smoke test PIE — passed
✅ WASD walk 240 cm/s, ✅ Shift hold → 320 cm/s, ✅ Mouse look raw + sensitivity 0.18, ✅ Head bob (subtle, decay при остановке), ✅ Pitch clamp +75°/-60°, ✅ FOV 75, ✅ No jump, ✅ Interact log placeholder, ✅ Journal log placeholder.

### Design package (locked)
- [PLAN.md](PLAN.md) — 14-day roadmap, 3 phases × ~5 days
- [.claude/context.md](.claude/context.md) — stack, naming, asset paths, decisions
- [docs/narrative/anomaly_choreography.md](docs/narrative/anomaly_choreography.md) — checklist (10-12 пунктов), 5-7 anomaly events, voice script (Silero TTS), 2 endings, tension curve
- [docs/levels/cvet-shum_layout.md](docs/levels/cvet-shum_layout.md) — 4-room layout (4×5 + 3×18 + 7×10 L-shape + 9×12), asset usage per room, Sketchfab hunt lists
- [docs/feel/atmosphere_plan.md](docs/feel/atmosphere_plan.md) — 4-layer sound stack, post-process timeline, camera/movement spec, TTS pipeline
- [docs/dev/bp_player_spec.md](docs/dev/bp_player_spec.md) — C++ ANoiseCharacter contract (workflow doc для Windows-стороны)
- [docs/dev/lvl_sluz_spec.md](docs/dev/lvl_sluz_spec.md) — Шлюз whitebox spec, 480 строк, 14 секций
- [WINDOWS_SETUP.md](WINDOWS_SETUP.md) — Windows-side checklist

---

## 5. Что планировали следующим (immediate)

### 5.1 Phase 1 Day 3 — active task

**Pivot:** вместо whitebox'а LVL_Sluz с нуля → **адаптировать Derelict Corridor sample level** под `LVL_Korridor` (наш центральный 18м коридор).

**Reasoning:** 262 Megascans ассета уже расставлены атмосферно в sample'е. Профессиональное освещение, нет розовых материалов, mood matches (industrial decay = horror tone). Экономия 1-2 дня на dressing.

**Pending от пользователя:**
1. Открыть Derelict Corridor sample level в Editor (`.umap` где-то в `Content/Megascans/` или `Content/DerelictCorridor/`)
2. PIE прохождение, прикинуть длину / разветвлённость
3. Top-view скриншот + описание layout'а → выкатить мне

**После того как user даст этот input** — я делаю спеку модификации:
- Подгонять длину под 18м (обрезать или extend'ить)
- Северный конец — дверь Б из Шлюза, южный — дверь Б' в Секцию В
- Manometer placeholder на 6м от северного входа, восточная стена
- Telephone niche на 12м от северного входа, восточная стена
- 5 fluorescent fixtures (если sample не покрывает)
- Audio Volume + SoundReverb_Korridor (MediumRoom, RT60=0.65s — atmosphere_plan §4)

### 5.2 Phase 1 Day 4-7 — план

| Day | Task |
|---|---|
| D4 | LVL_Sluz whitebox по [docs/dev/lvl_sluz_spec.md](docs/dev/lvl_sluz_spec.md) — спека готова, выполнять напрямую |
| D5 | LVL_SektsiyaV whitebox L-shape (7×7 основной + 4×3 крыло) — нужна спека от меня |
| D6 | LVL_ControlRoom whitebox 9×12, U-pult placeholder — нужна спека |
| D7 | Door connections / Level Streaming — single map streaming или 4 sublevels (решить когда дойдём) |

### 5.3 Backlog для Phase 1 D8 polish
- `ANoiseFluorescentLight` C++ class (flicker logic) — TODO §11 в lvl_sluz_spec, отдельный класс с timer-based flicker per atmosphere_plan §3 (0.143 Hz base, ±30% jitter, 0.05s dark frame)
- Audio Layer 1 ambient (60Hz hum + fluorescent buzz + ventilation hum) — Sonniss/Freesound CC0 поиск + UE5 import
- Sketchfab manual hunt — 5-8 props (locker, soviet rotary phone, manometer detailed, control panel, A4 frames)

---

## 6. Phase 2-3 (overview, не immediate)

### Phase 2 (D5-D9) — Core loop
- Interaction system — `UNoiseInteractionComponent` (trace + outline + E-press)
- Checklist UI — paper journal в руках, Tab toggle, UMG widget
- Procedure system — DT_Procedures + state machine
- 3 scripted anomalies: MANOMETER_DRIFT, TELEPHONE_NEVER_ANSWERS, JOURNAL_ENTRY_BY_YOU
- Silero TTS Russian — Python script для batch generation announcement-голоса
- Enhanced Input migration — IA_Move, IA_Look, IA_Interact, IA_CautiousJog, IA_OpenJournal + IMC_Default. Удалить legacy mappings из DefaultInput.ini.

### Phase 3 (D10-D14) — Polish + finale
- Final control panel encounter, 2 endings
- Visual polish ramp по timeline (chromatic aberration, film grain, saturation, temp shift на минутах 33-34)
- Audio mix всех 4 layers + 60Hz hum пропадает на 3.2 сек (минута 20-30) — главное аудио событие
- Windows packaged build

---

## 7. Known problems / gotchas

### 7.1 Build / dev environment
- **VS 2026 (Insider) installed** alongside VS 2022 — UE5 должен использовать VS 2022. Если default ассоциация на 2026 → ПКМ на `noise.sln` → Open with → Visual Studio 2022.
- **VS Build Solution** падает на `SetEnv` (env var too long) даже на VS 2022 — это Windows env block нагружен. Workaround: **двойной клик `noise.uproject`** запускает UBT напрямую, минуя MSBuild SetEnv. Использовать этот path для compile, не VS Build.
- **`Live Coding` hot reload** в UE5 5.7 иногда glitchy. После C++ изменений — закрыть Editor → Build → reopen. Безопаснее чем live reload.
- **OneDrive** = file locks при sync. Проект перенесён в `C:\Unreal Projects\noise` чтобы избежать. **Не возвращать в OneDrive.**

### 7.2 Asset / git
- **`.gitignore`** скушал: `Saved/`, `Intermediate/`, `Binaries/`, `DerivedDataCache/`, `.vs/`, `*.sln`, `Backup/`, `UpgradeLog.htm`, `shadertoolsconfig.json`. Если что-то из этого появилось в `git status` — gitignore поломался, расследовать.
- **LFS** обязателен для `.uasset`/`.umap`. Verify через `git lfs status` или `git check-attr -a <file>` — должно быть `filter: lfs`.
- **`Content/NiagaraExamples/Materials/MasterMaterials/M_SmokeAndFire_Sprites_Backup.uasset`** — легитимный asset с "Backup" в имени. Не путать с `Backup/` папкой UE5 (в gitignore).
- **Никаких rename/move через Windows Explorer** для already-tracked `.uasset`/`.umap` — только через UE5 Editor + Fix Up Redirectors. Иначе break references.
- **Editor должен быть закрыт** перед git commit'ом — некоторые `.uasset` дописываются на диск только при close.

### 7.3 Code / build
- **Subfolders в `Source/noise/`** ломают `#include "Path/X.h"` в UE5.7 без явных `PrivateIncludePaths` в Build.cs. Структура flat. Если когда-то понадобится subfolder split — добавлять в Build.cs.
- **`NOISE_API` macro** генерится из module name "noise". Если переименовать module — все классы тоже надо обновить.
- **`TObjectPtr<...>`** — UE5.5+ idiom для UPROPERTY pointers. Использовать вместо raw `T*` для UPROPERTY'и.

### 7.4 Workflow
- **Force-push был использован однократно** для clean reset (OneDrive→C:\Unreal Projects/). Избегать в будущем — теряет history.
- **User commit message "123"** в `9cd614e8` — bad pattern, напоминать про meaningful commit messages.
- **Maps & Modes** UE5 5.7 saves в `Config/DefaultEngine.ini` (не DefaultGame.ini). Если ищешь EditorStartupMap — там.

---

## 8. Куда смотреть в первую очередь

**Order of reading в новой сессии:**

1. **Этот файл (HANDOFF.md)** — снимок состояния
2. **[PLAN.md](PLAN.md)** — 14-day roadmap, milestone milestones, current phase
3. **[.claude/context.md](.claude/context.md)** — stack, conventions, decisions, asset paths
4. **`git log --oneline -10`** — последние commits, чтобы понять что недавно делалось
5. **[docs/dev/lvl_sluz_spec.md](docs/dev/lvl_sluz_spec.md)** — если active task = level building
6. **[docs/dev/bp_player_spec.md](docs/dev/bp_player_spec.md)** — если active task = player feel или C++ Character
7. **[docs/feel/atmosphere_plan.md §1](docs/feel/atmosphere_plan.md)** — single source of truth на feel параметры
8. **[docs/levels/cvet-shum_layout.md](docs/levels/cvet-shum_layout.md)** — 4-room spatial spec
9. **[docs/narrative/anomaly_choreography.md](docs/narrative/anomaly_choreography.md)** — checklist + anomalies + endings + voice script
10. **[WINDOWS_SETUP.md](WINDOWS_SETUP.md)** — если новая Windows machine setup нужен

**Source code orientation:**
- `Source/noise/NoiseCharacter.{h,cpp}` — actual player code (200 строк)
- `Source/noise/NoiseGameModeBase.{h,cpp}` — GameMode (12 строк)
- `noise.uproject` — Modules + Plugins
- `Config/DefaultEngine.ini` — render + audio
- `Config/DefaultInput.ini` — WASD + Mouse + E/Shift/Tab mappings

**Memory** (Linux side):
- `~/.claude/projects/-home-azukki-cvet-shum/memory/MEMORY.md` — Claude memory pointers
- `~/.claude/projects/-home-azukki-cvet-shum/memory/project_ue5_workflow.md` — UE5 on Windows note

---

## 9. Workflow conventions

- **Code only via `unreal-engineer` agent** (per global CLAUDE.md analog rule for UE5 — все code-files через agent, не самостоятельно)
- **Что МОЖНО делать самому без agent**: читать файлы, `grep`/`find`, `git status/diff/log/commit`, редактировать markdown-доки + PLAN.md + context.md, коммитить + пушить уже написанный agent'ом код
- **Никаких `git add -A`** — specific files only (зацепит `Saved/`/`Intermediate/`)
- **Commit format:** `phase1 d2: ...`, `ADR: ...`, `feat: ...`, `fix: ...`. Co-Authored-By не используем (matches user's existing style)
- **Закрывать UE Editor** перед commit'ом
- **`git pull --rebase`** перед push, не plain `pull` (избегаем merge commits)
- **Pull через GitHub Desktop** на Windows — для повседневной работы, CLI git только для diverged history / force-push / rebase
- **MCP для UE** — deferred до Phase 2+, не настраиваем сейчас

---

## 10. Open questions / pending input

### От пользователя:
- **Derelict Corridor sample level** — путь к `.umap`, top-view screenshot, описание layout'а. С этой информацией → пишу спеку модификации под LVL_Korridor.

### Будут разрешаться позже:
- **Walk speed 240** — recalibrate playtest D5-6 (240 → 300 если тягомотина, не 480)
- **EV100 = 10** в PostProcess Volume — calibrate playtest'ом, atmosphere_plan §3 предполагает 10-12
- **Level streaming vs single map** — решить на D7 когда все 4 levels готовы
- **MCP для UE** — Phase 2+, ChiR24/Unreal_mcp как OSS-кандидат если зрелость подойдёт

---

*Этот файл — living document. Обновлять при крупных decisions / commit'ах. Сохранять в репо (он git-tracked).*
