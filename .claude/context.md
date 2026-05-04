# ЦВЕТ-ШУМ — Project Context

**Display name:** ЦВЕТ-ШУМ
**Codename for filenames:** `noise`
**Engine:** Unreal Engine 5.5+ (5.7 для совместимости с Niagara Examples Pack UE5.7)
**Repo:** /home/azukki/cvet-shum
**Branch:** main
**Build target:** Windows packaged build
**Cost budget:** $0 (всё owned или free)
**Length budget:** 14 дней (3 фазы × ~5 дней)

---

## UE5 setup

- **UE5 binary location:** на Windows-машине пользователя. Linux dev box (этот) **не имеет UE5 binary** — headless validation (`CompileAllBlueprints`) невозможна. Workflow: Linux = code/config/git, Windows = open project in UE5 Editor + manual playtest. Git — мост между средами.
- **UE5 version:** 5.5 (минимум; пользователь может bump до 5.6/5.7 через `EngineAssociation` в `.uproject`). Niagara Examples Pack 5.7 потребует engine 5.7 — bump перед import'ом.
- **Plugins required (в `.uproject`):**
  - `Niagara` (default ON в UE5)
  - `EnhancedInput` (UE5.5+ default — для Phase 2 Input Actions)
  - `ModelingToolsEditorMode` (для whitebox layout в Phase 1)
- **Movement:** стоковый UE5 `Character` + custom `BP_Player`. **GASP (Game Animation Sample Project) не используем** — это third-person motion matching showcase, для slow-walk first-person horror без traversal/jump/crouch overkill (~5 GB ассетов с 0% полезной поверхности).
- **Project file:** `noise.uproject` (создан Phase 1 Day 1 — text-only setup на Linux)

---

## Owned paid asset packs (in user's Fab library, локально скачаны)

- **Cold War Underground Bunker** — https://www.fab.com/listings/5a4b7b4c-67cd-47ff-bdd5-6373e7b62dc6
  - 152 уникальных меша, brutalist Soviet bunker
  - **Использование:** все 4 помещения (стены, потолки, двери, fluorescent fixtures, server cabinets). Покрывает control room, utility, коридор, шлюз, секцию В.

**ADR (2026-05-04):** Soviet/Post Soviet Abandoned World pack недоступен (отвалился из Fab Library пользователя). Заменён на: Cold War Bunker (двери, базовые модули) + Sketchfab CC-BY (locker, operator desk, chair, штучные props) + Material decals от PolyHaven CC0 / AmbientCG (peeling paint variation для крыла Секции В, warning signs на дверях). См. `docs/levels/cvet-shum_layout.md` per-room asset usage maps.

## Free anchor assets

- **Derelict Corridor Megascans Sample** (постоянно free) — https://www.fab.com/listings/d825ff9c-77da-45a4-9619-c989e6dfdda6
  - 262 фотосканированных industrial assets (трубы, кабельные короба, распредщиты, дебрис, мебель + 11 environment materials + 15 поверхностей + 34 decals)
  - **Использование:** деталеобразование коридора + секции В + всех мест где нужны industrial detail props
- **Niagara Examples Pack UE5.7** (free на Fab) — https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600
  - 50+ Niagara-систем от Epic: туман, пыль, sparks, smoke, explosions
  - Требует engine 5.7 — bump `EngineAssociation` перед import'ом
- **Megascans Free Tier** — стартер-пак ~1500 ассетов

## Free supplementary

- **PolyHaven** — https://polyhaven.com/textures (CC0, есть UE5 plugin)
- **AmbientCG** — https://ambientcg.com/ (CC0)
- **Sketchfab CC-BY** — manual hunt для штучных советских props (~5-8 моделей: detailed control panels, manometer dial 30cm, soviet rotary telephone, soviet desk chair, laminated A4 регламенты)
- **Sonniss GDC archive** — https://gdc.sonniss.com/ (royalty-free, категория `industrial_mechanical`)
- **Freesound.org** (CC0 фильтр) — для interactive sounds
- **BBC SFX archive** — https://sound-effects.bbcrewind.co.uk/ (RemArc лицензия — non-commercial OK для personal проекта)
- **Zapsplat** — footsteps, mechanical clicks

---

## Russian text rules

- UTF-8 везде
- UMG fonts: использовать с гарантированной Cyrillic glyph coverage — **Roboto / Inter / NotoSans / Source Sans Pro**. **НЕ** использовать Calibri/Verdana по умолчанию.
- Hardcoded строки в Blueprint — `FText` (для возможной локализации). FString допустимо для placeholder во время прото.
- All player-facing text — на русском, formal Soviet institutional tone.

## Voice generation pipeline (Silero TTS)

- **Tool:** Silero TTS v5 (MIT) — https://github.com/snakers4/silero-models
- **Model:** `v5_cis_base` (male voice для диктора)
- **Language:** русский с автоматической постановкой ударений и разрешением омографов
- **Workflow:**
  1. Python script для batch generation (заданы тексты в `docs/narrative/anomaly_choreography.md` секция 3)
  2. Output: `.wav` files в `Content/Audio/Voice/` с naming `S_announcement_NN.wav`
  3. UE5: Sound Cue с Reverb preset для radio broadcast feel (внутренняя трансляция через speaker)
- **Quality benchmark:** UTMOS ~3.04 при эталоне 3.08 — minimal разница, продакшн-grade

---

## Naming conventions (UE5 community standard)

- `BP_<Name>` — Blueprint Class
- `WBP_<Name>` — Widget Blueprint
- `M_<Name>` / `MI_<Name>` / `MF_<Name>` — Material / Instance / Function
- `T_<Name>` — Texture
- `SK_<Name>` / `SM_<Name>` — Skeletal Mesh / Static Mesh
- `A_<Name>` / `AM_<Name>` / `ABP_<Name>` — Animation Sequence / Montage / Blueprint
- `S_<Name>` — Sound (`S_ambient_60hz_drone`, `S_announcement_01`, `S_phone_dial_tone`)
- `NS_<Name>` — Niagara System
- `DT_<Name>` — Data Table
- Уровни: `LVL_Sluz`, `LVL_Korridor`, `LVL_SektsiyaV`, `LVL_ControlRoom` (или single map с streamed sub-levels — решить в Phase 1)

## Asset directory structure

**Принцип:** третьесторонние паки живут в КОРНЕ `Content/` под их оригинальным именем папки (как `.uasset` хранит `/Game/<original>/...` внутри). Не перемещать через Move — каждый Move риск broken refs. Наша authored работа — в организованных папках.

```
Content/
├── ColdWarBunker/          # ← pack at root (preserves /Game/ColdWarBunker/... refs)
├── Megascans/              # ← Derelict Corridor pack (preserves /Game/Megascans/... refs)
├── NiagaraExamples/        # ← Niagara Examples Pack
│
├── Audio/                  # ← наша работа
│   ├── Ambient/            # Layer 1: 60Hz drone, fluorescent buzz, ventilation hum
│   ├── Reactive/           # Layer 2: footsteps, paper, pen, door
│   ├── Critical/           # Layer 3: phone tones, piano note, sirena
│   └── Voice/              # Silero TTS output
├── Blueprints/             # ← наша работа
│   ├── Player/             # BP_Player, BP_FirstPersonController
│   ├── Interactables/      # BP_Journal, BP_Manometer, BP_Telephone, BP_ControlPanel, BP_DoorLabel
│   └── Anomalies/          # Scripted event Blueprints
├── Data/                   # DT_Procedures
├── Sketchfab/              # ← наш manual hunt (мы контролируем path)
├── Levels/                 # .umap files
├── Materials/              # ← наши PolyHaven, AmbientCG processed
├── PostProcess/            # PP_Volume settings, ramp curves
├── UI/                     # WBP_Journal, WBP_OutlineHUD
└── VFX/                    # ← наши Niagara systems (изначально в NiagaraExamples)
```

**Конкретные имена папок паков** (что лежит в корне) определяет источник; наблюдать через Content Browser после migrate'а. Не переименовывать.

---

## Workflow

- **Parallel impl:** через `Agent({isolation: "worktree"})` — Claude автоматически создаёт worktree per agent. См. PLAN.md "Parallel streams" — 3 streams (Level / Mechanics / Audio).
- **Validation:** UE5 binary не доступен на Linux. Проверка корректности проекта — через manual open в UE5 Editor на Windows (Editor покажет ошибки в Output Log при загрузке). Headless `CompileAllBlueprints` доступен только если пользователь запустит локально на Windows из cmd.
- **Smoke test:** manual playtest на Windows после Phase 1 D4 milestone (PIE — Play In Editor, или standalone build).
- **Git protocol:**
  - `git pull --rebase` перед commit'ом
  - `git add <specific files>` — НЕ `git add -A` (зацепит /Saved/ /Intermediate/ /DerivedDataCache/)
  - Commits как `zulesss <viktorpostemskij@gmail.com>` (см. memory `user_git_identity.md`)
  - Karpathy self-check на >20 строк текстового кода (см. unreal-engineer agent в `~/.claude/agents/`)
  - Push после commit'а — рассмотреть post-commit hook аналогично vgate'у если будет полезно

---

## Reference design docs

- `docs/narrative/anomaly_choreography.md` — checklist (10-12 пунктов), anomaly events (5-7), voice script, finale 2 endings, tension curve
- `docs/levels/cvet-shum_layout.md` — 4-room layout (Шлюз / Коридор / Секция В / Control Room), asset usage per room, key feature placement, lighting zones, Sketchfab hunt list
- `docs/feel/atmosphere_plan.md` — camera/movement (FOV 75, walk 240 cm/s), 4-layer sound stack, post-process timeline, interaction feedback, TTS pipeline

---

## Resolved design decisions (locked 2026-05-04)

5 open questions из PLAN.md — закрыты пользователем:

1. **Финальный пульт — без фиксации чисел в журнале.** Игрок впишет 14З/2К в свободном тексте если захочет, но system не подсвечивает диссонанс при финальном осмотре. Paranoia route: игрок сомневается видел ли он 14 или 11. Hardcore horror call.
2. **Дверь Секция В → Control Room — always open.** Никаких trigger-conditions. Все двери (кроме финального lock'а В после 4.2) проходимы в обе стороны на всём протяжении игры.
3. **Walk speed — 240 cm/s lock на Day 1.** Recalibrate playtest D5-6: если ощущается тягомотиной → 300, не 480. Shift = 320 cm/s.
4. **65-сек telephone UX cue — текст пункта 3.2 регламента.** Прямая инструкция: "не вешать трубку до ответа диспетчера". Никаких UI-prompt'ов, toast'ов или indicator'ов.
5. **CLOCK_WALKS_BACK — оставить тихой.** Опциональная anomaly без процедурного крючка. Часть аномалий должна оставаться на periphery — это часть дизайна. Не каждый игрок должен заметить всё.
