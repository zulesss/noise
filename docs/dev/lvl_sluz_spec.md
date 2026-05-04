# LVL_Sluz — Whitebox + Lighting + Reverb Spec (UE5.5+)

**Version:** 1.0 (2026-05-04, Phase 1 Day 3)
**Engine:** UE5.5+ (5.7 если bumped)
**Reference (single source of truth):** [`docs/levels/cvet-shum_layout.md §2.1`](../levels/cvet-shum_layout.md) — geometry, asset usage, key features, lighting design. [`docs/feel/atmosphere_plan.md §3 + §4`](../feel/atmosphere_plan.md) — post-process baseline, reverb zones.

---

## 0. Header

**Цель:** первый игровой level Шлюз — компактная rectangular комната 4×5×3м с одним fluorescent fixture на потолке. Player spawn'ится у северной стены, видит дверь А впереди (юг), двигается в сторону неё. Atmospherically: institutional, чуть сжатое, "ещё нормальное" пространство (см. layout §2.1.a).

**Asset target:** `Content/Levels/LVL_Sluz.umap`

**Phase 1 Day 3 milestone.** Время на исполнение: ~30-60 мин на whitebox + ~30 мин на lighting/reverb/post-process. Dressing pass (замена placeholder boxes на реальные меши Cold War Bunker) — Phase 1 D4-D6.

**Scope этого документа (НЕ scope):** только whitebox + lighting + reverb + placeholder props + smoke test PIE. Никаких interaction objects (Phase 2), никаких anomaly triggers (Phase 2), никаких финальных мешей.

---

## 1. Pre-requisites

Перед тем как начать — проверь:

- [ ] `BP_Player` (extends `ANoiseCharacter`) и `BP_GameMode_Default` (Default Pawn = `BP_Player`) уже работают (Phase 1 D2 завершён, см. `docs/dev/bp_player_spec.md`).
- [ ] Cold War Bunker pack импортирован в `Content/ColdWarBunker/` (152 mesh). Проверка: открой любой `SM_*` из этой папки в редакторе — материал не pink, mesh видим.
- [ ] Derelict Corridor Megascans в `Content/Megascans/` (262 industrial assets) — для Phase 1 D3 не критично (используем placeholder boxes вместо truly-finished props), но pack должен быть.
- [ ] UE5.7 Editor (или 5.5+) запускается без compile errors. `Source/noise` модуль скомпилирован.
- [ ] `Modeling Tools` plugin enabled — Edit → Plugins → search "Modeling Tools" → ON. Restart editor если только что включил.

---

## 2. Создать LVL_Sluz.umap

1. **File → New Level → Empty Level** (НЕ Basic — Шлюз это interior bunker без неба, default Sky Sphere / Sun Light не нужны и будут мешать).
2. **File → Save Current Level As** → Path: `Content/Levels/` → Name: `LVL_Sluz` → Save.

Уровень создан как `Content/Levels/LVL_Sluz.umap`.

---

## 3. World Settings

**Window → World Settings** (если panel'и нет в layout — Window menu, обычно справа от Details).

- **GameMode Override:** оставить пустым (`None`). Используется global default из Project Settings → Maps & Modes (`BP_GameMode_Default` с `BP_Player` per Phase 1 D2).
- **Kill Z:** `-500` (опционально, защита если игрок упадёт через пол из-за collision bug — re-spawn).
- **World Composition / Streaming Volumes:** не трогать. Phase 1 — single map, no streaming.
- **Lightmass Settings:** не запекаем (Lumen real-time GI — см. atmosphere_plan §3). Игнорируем эти поля.

---

## 4. Whitebox геометрия — 4×5×3м rectangular

**КРИТИЧЕСКИ ВАЖНО:** UE5 единицы — сантиметры. **4м × 5м × 3м = 400 × 500 × 300 unreal units.**

### Coordinate system convention

- **X** = east-west (X+ = east)
- **Y** = north-south (Y+ = south, Y- = north)
- **Z** = up

Per layout §2.1.b — Шлюз самая северная комната, вход с улицы с севера, выход (дверь А) на юг в коридор. Player spawn у северной стены лицом к двери А.

Origin (0, 0, 0) ставим в **центр пола Шлюза**. Это упрощает arithmetic — стены симметрично ±200 / ±250.

### Опция А — Modeling Tools (рекомендую)

1. Top toolbar → Selection mode dropdown (default "Selection") → **Modeling**.
2. В появившейся Modeling panel слева → **Create** tab → **Box** primitive.
3. Build каждый box ниже. После placement — Esc для выхода из tool, в Details panel выставить точные Location и Size.

| Element | Size (X × Y × Z) | Location (X, Y, Z) |
|---|---|---|
| Floor | 400 × 500 × 10 | (0, 0, 0) — top surface на Z=5 |
| Ceiling | 400 × 500 × 10 | (0, 0, 300) |
| North Wall | 400 × 10 × 300 | (0, -250, 150) |
| South Wall (с вырезом) | 400 × 10 × 300 | (0, +250, 150) |
| East Wall | 10 × 500 × 300 | (+200, 0, 150) |
| West Wall | 10 × 500 × 300 | (-200, 0, 150) |

**Важно про Box origin:** Modeling Tools Box по умолчанию origin = bottom center. Для floor — Location.Z = 0 значит pivot на полу, mesh extends 0..10 по Z. Если у тебя Box origin = mesh center, скорректируй: Floor Location.Z = 5, Ceiling Location.Z = 295, walls Location.Z = 150 (это уже center-based значения в таблице выше, проверь поведение Box tool — UE5.5 default = bottom center, перепроверь в твоей версии).

`// TODO: verify Box pivot default in UE5.7 Modeling Tools — может быть center, может bottom`

### Опция Б — BSP brushes (если не любишь Modeling Tools)

1. Place Actors panel → Geometry → Box Brush.
2. Drag в viewport, в Details:
   - **Brush Settings → X / Y / Z:** размеры (см. таблицу выше)
   - **Transform → Location:** позиция

Тот же набор 6 brushes. Преимущество BSP — простота, недостаток — в современном UE5 Modeling Tools предпочтительнее (можно конвертить в static mesh, текстурить как обычный asset).

### Material assignment

После всех 6 boxes — выделить все в World Outliner (Shift+click) → Details → Materials → assign placeholder. Варианты:

- `Engine/EngineMaterials/M_Default` (стерильный шахматный паттерн)
- `Engine/EngineDebugMaterials/M_Brushed_Metal_01` или похожее industrial
- Любой `M_*` из `Content/ColdWarBunker/Materials/` (если есть concrete-like material — это даст immediate atmospheric vibe, но не обязательно для whitebox)

**Цель whitebox — увидеть форму комнаты. Финальный visual через mesh dressing — Phase 1 D4-D6.**

---

## 5. Дверь А (вырез в южной стене)

Per layout §2.1.b/d — дверь А в южной стене ведёт в коридор. Размер дверного проёма: **0.9м × 2.0м** (90 × 200 cm). По центру X, нижний край на полу.

### Опция А — Boolean Subtract через Modeling Tools (рекомендую)

1. Modeling mode → **Mesh Boolean** tab → **Subtract**.
2. Создай вспомогательный Box: 12 × 20 × 200, location (0, +250, 100). Это "вычитающий" объём — должен быть толще стены (12 > 10) чтобы гарантированно прорезать насквозь, шириной 90 cm (X), высотой 200 cm (Z от пола). Y=+250 точно в стене (чуть толще для запаса).

   Wait — пересчитаю: дверь 90 cm wide (X) × 200 cm tall (Z), стена толщиной 10 cm (Y). Tool box: **90 × 20 × 200**, location (0, +250, 100). Box pivot center → centered на (0, +250, 100), extends X: -45..+45, Y: +240..+260, Z: 0..200. Wall extends Y: +245..+255. Box покрывает Y: +240..+260 — gehört сквозь стену. ✓

3. В Mesh Boolean panel → Tool Mesh = вспомогательный Box, Target Mesh = South Wall → Subtract → Apply.
4. Удали вспомогательный Box (если не auto-removed).

Результат: South Wall — single mesh с проёмом 90×200 cm в центре.

### Опция Б — три отдельных wall pieces (без boolean)

Если boolean не получается (Modeling Tools sometimes flaky на overlapping geometry):

| Piece | Size (X × Y × Z) | Location |
|---|---|---|
| Left segment | 155 × 10 × 300 | (-122.5, +250, 150) |
| Right segment | 155 × 10 × 300 | (+122.5, +250, 150) |
| Header (над проёмом) | 90 × 10 × 100 | (0, +250, 250) |

Math check: total X coverage = 155 + 90 (gap) + 155 = 400. ✓ Header bottom Z = 200 (= door top), top Z = 300. ✓

### Visual placeholder для двери А (не collision, атмосферный prop)

После выреза — на месте проёма поставить плоский placeholder (это будет реальная Cold War Bunker door в Phase 1 D4):

- Box: 90 × 5 × 200
- Location: (0, +250, 100)
- Material: что-то отличающееся от стен — например `M_Wood` из Engine, или просто другой цвет
- **Collision:** Set to `NoCollision` в Details → Collision → Collision Presets, чтобы можно было пройти насквозь. В Phase 1 D3 нам нужно физически выйти из комнаты для smoke test'а.

`// TODO: D4 — заменить на SM_Door_*. из Content/ColdWarBunker/Meshes/, добавить proper collision и interaction в Phase 2`

---

## 6. PlayerStart

Per layout §2.1.f — spawn у северной стены, лицом к двери А (на юг).

1. **Place Actors panel** (Window → Place Actors если скрыт) → Basic → drag **Player Start** в viewport.
2. Details panel:
   - **Transform → Location:** `(0, -200, 100)`
     - X=0 — по центру комнаты
     - Y=-200 — у северной стены (стена на Y=-250, отступ 50 cm)
     - Z=100 — на уровне глаз (PlayerStart pivot обычно у пола, Character spawn'ится с capsule center на этой Z; стандартный UE5 Character capsule half-height ~88 cm, eye height ~1.6м от capsule center → reasonable)
   - **Transform → Rotation:** `(0, 90, 0)` — Pitch=0, **Yaw=90**, Roll=0
     - Yaw 0 = +X (east). Yaw 90 = +Y (south). Игрок смотрит на дверь А. ✓

`// TODO: verify Character spawn Z offset — stock UE5 Character может spawn'иться с capsule snap to ground; PlayerStart Z=100 может стать Z=88 после snap. Если игрок спавнится в потолке/полу — adjust Z до 88 или 90`

---

## 7. Lighting — 1 fluorescent + ambient

Per layout §2.1.e + atmosphere_plan §3.

### 7.1 Spot Light (fluorescent fixture, центр потолка)

1. Place Actors → Lights → **Spot Light** → drag в viewport.
2. Details panel:
   - **Transform → Location:** `(0, 0, 290)` — центр потолка, чуть ниже фактической Z=295
   - **Transform → Rotation:** `(0, -90, 0)` — Pitch=-90 (направлен в пол), Yaw=0, Roll=0

     `// TODO: verify Pitch sign convention в UE5.7. В UE5 default — отрицательный Pitch = смотреть вниз. Если свет уходит в потолок — поменяй на +90.`

   - **Light section:**
     - **Intensity:** `800` (units = candelas, default Inverse Squared Falloff). Placeholder для тестирования — calibrate playtest'ом.
     - **Light Color:** включи **Use Temperature** → **Temperature = 4000** (K). Per layout §2.1.e — 4000K cool white.
     - **Attenuation Radius:** `600` (вся комната покрывается)
     - **Source Radius / Soft Source Radius:** `5` (soft shadow для "fluorescent tube" feel)
     - **Inner Cone Angle:** `30°`
     - **Outer Cone Angle:** `60°`
   - **Light → Mobility:** **Stationary** (для Lumen + flicker animation в будущем — Stationary даёт static shadows + dynamic intensity changes).
   - **Light → Cast Shadows:** ON.

### 7.2 Sky Light (fill ambient — индирект indoor)

1. Place Actors → Lights → **Sky Light** → drag в viewport.
2. Details:
   - **Source Type:** SLS Captured Scene
   - **Recapture Scene** button — кликнуть после placement (recapture при changes).
   - **Intensity:** `0.3` (низкий ambient — мы в interior, не open world)
   - **Lower Hemisphere Is Black:** True (нет неба под полом)
   - **Mobility:** Movable (для Lumen real-time updates)

Sky Light нужен чтобы Lumen имел fill для теневых углов (без него углы у пола могут быть absolute black без bounce).

### 7.3 Post Process Volume (unbound — global для всего level'а)

Per atmosphere_plan §3 baseline.

1. Place Actors → Volumes → **Post Process Volume** → drag в viewport (точное расположение неважно).
2. Details:
   - **Post Process Volume Settings → Unbound:** ✓ True (применяется ко всему level'у, не только внутри volume)
   - **Priority:** 0 (default)

#### Lens → Exposure
- **Metering Mode:** Manual (no auto-adapt — atmosphere_plan §3)
- **Exposure Compensation:** 0
- **Apply Physical Camera Exposure:** False (override через Manual EV100)
- **Manual EV100:** `10` (interior baseline — calibrate в playtest)

#### Lens → Bloom
- **Method:** Convolution
- **Intensity:** 0.8
- **Threshold:** 1.2

#### Lens → Chromatic Aberration
- **Intensity:** 0 (Phase 1 baseline — ramp начнётся в Phase 3 timeline)

#### Lens → Film Grain
- **Intensity:** 0.15
- **Size:** 1.0

#### Color Grading → Global → Saturation
- (1, 1, 1, 1) (Phase 1 baseline)

#### Color Grading → White Balance
- **Temp:** 4200 K
- **Tint:** 0

#### Lens → Image Effects → Vignette Intensity
- `0.35`

   `// TODO: verify "Vignette Roundness" UI location в UE5.7 — может быть в Camera vs Lens section. Default value = 0.5 окей.`

#### Rendering Features → Depth of Field

Per atmosphere_plan §3 — Circle DOF, f/2.8, focus 180 cm. Для Phase 1 D3 whitebox — **не настраиваем DOF** (отвлечёт от тестирования geometry/lighting). Добавим в D4-D5 dressing pass.

`// TODO: D4-D5 — DOF setup per atmosphere_plan §3`

---

## 8. Audio Volume — Reverb (Шлюз = SmallRoom)

Per atmosphere_plan §4 reverb zones table — Шлюз: SmallRoom preset, RT60 = 0.35s, Pre-delay = 8ms, Low freq rolloff -3 dB @ 200 Hz.

### 8.1 Создать Reverb Effect asset

1. Content Browser → перейти в `Content/Audio/Reverb/` (создать папку если нет).
2. ПКМ в browser → **Audio → Effects → Reverb Effect** → Name: `SoundReverb_Sluz`. Save.
3. Open `SoundReverb_Sluz`. Settings:

   | UE5 field | Value | Mapping к atmosphere_plan §4 |
   |---|---|---|
   | Density | 1.0 | Default (full density) |
   | Diffusion | 1.0 | Default |
   | Gain | 0.32 | -10 dB rough (placeholder) |
   | Gain HF | 0.89 | Slight HF rolloff |
   | Decay Time | 0.35 (s) | RT60 = 0.35s ✓ |
   | Decay HF Ratio | 0.83 | Default-ish |
   | Reflections Gain | 0.05 | Faint early reflections |
   | Reflections Delay | 0.007 (s) | ~7 ms — близко к 8 ms |
   | Late Gain | 0.5 | Default |
   | Late Delay | 0.011 (s) | ~11 ms |
   | Air Absorption Gain HF | 0.994 | Default |

   `// TODO: verify exact UE5.7 Reverb Effect field names. UE5 Reverb Effect использует `FAudioReverbEffect` struct fields — некоторые имена могут отличаться (Gain vs Volume vs Wet Level). Цель: Decay Time=0.35s, Pre-delay (Reflections Delay)=~8ms, room feel "small". Если Low freq rolloff -3 dB @ 200 Hz нет direct slider — ок, это nice-to-have, не блокер D3.`

4. Save asset.

### 8.2 Audio Volume

1. Place Actors → Volumes → **Audio Volume** → drag в viewport.
2. Details:
   - **Brush Settings → Brush Shape:** Box
   - **Brush Settings → X / Y / Z:** `380 × 480 × 290` (немного меньше комнаты чтобы не торчал в стены)
   - **Transform → Location:** `(0, 0, 145)` (centered внутри комнаты)
   - **Audio Volume → Reverb Settings → Reverb Effect:** assign `SoundReverb_Sluz`
   - **Audio Volume → Reverb Settings → Volume:** 0.7 (wet level)
   - **Audio Volume → Reverb Settings → Fade Time:** 1.5 (per atmosphere_plan §4 — crossfade between zones)
   - **Audio Volume → Priority:** 1 (выше default 0)

Player в этом volume → reverb включается. Player вне volume (за дверью) → reverb fade out 1.5s.

`// TODO: D6+ когда LVL_Korridor готов — оба Audio Volume будут overlapping у двери А; убедиться что Priority правильно расставлен (Шлюз=1, Корридор=1, можно по Priority выше у того в чьей зоне игрок strictly inside).`

---

## 9. Placeholder props

Per layout §2.1.c/d — placeholder boxes с разными materials. Реальные меши заменим в Phase 1 D4-D6.

**Coordinate reminder:** комната 400×500 cm, origin центр пола. Север Y=-250, юг Y=+250, запад X=-200, восток X=+200.

### 9.1 Locker (северная стена, левее входа)

- Box: 50 × 40 × 180 cm (W×D×H)
- Location: `(-150, -230, 90)` — у северной стены (Y=-230 = в 20 cm от стены), слева (X=-150), pivot center → bottom Z=0
- Material: M_BrushedMetal или просто distinct color tag
- **Final asset (Sketchfab hunt):** Soviet double metal locker — см. layout §2.1.g item 3.

### 9.2 Стол у западной стены

- Box: 80 × 60 × 75 cm
- Location: `(-170, 50, 37.5)` — у западной стены (X=-170), южнее центра комнаты (Y=+50)
- Material: M_Wood или industrial metal placeholder
- **Final asset:** Soviet metal desk — layout §2.1.g item 1.

### 9.3 Paper journal на столе

- Box: 20 × 30 × 3 cm
- Location: `(-170, 50, 76.5)` — на столе (Z=75 + half-thickness 1.5 = 76.5)
- Material: M_Paper или яркий цвет (visibility для smoke test)
- **Final asset:** Phase 2 — interactive `BP_Journal` (extends `ANoiseInteractable`, который ещё не создан).

### 9.4 Регламент A4 на восточной стене (REGULATION_SHEET_01)

Per layout §2.1.d — на восточной стене, на уровне глаз (1.6м от пола), прямо по линии взгляда от двери А.

Wait — линия взгляда от двери А (юг, Y=+250) направлена на север. Восточная стена — справа от player'а когда он входит. Per layout §2.1.d "Это первый интерактивный объект который игрок видит при входе" — игрок входит **из улицы** (с севера), смотрит на юг к двери А; восточная стена справа от него. Регламент **на восточной стене, на уровне глаз**. ✓

- Plane (Plane primitive в Modeling Tools, или Box очень тонкий): 21 × 1 × 30 cm (A4 portrait)
- Location: `(+195, -100, 160)` — у восточной стены (X=+195, толщина стены 10 → внутренняя поверхность X=+195), Y=-100 (немного северо-центрально), Z=160 (уровень глаз)
- Rotation: `(0, 0, 0)` (плоскость лицом на запад — т.е. в комнату)
- Material: M_Default + текст-decal "REGULATION_SHEET_01" placeholder
- **Final asset:** UI Material с текстом регламента + laminated frame mesh — Sketchfab hunt §2.1.g item 2 (Phase 2).

### 9.5 Дверь А visual placeholder

Уже описана в §5 (Box 90×5×200 на месте проёма с NoCollision).

---

## 10. Smoke test в PIE

### 10.1 Установить как default map для теста

Project Settings → **Project → Maps & Modes**:
- **Editor Startup Map:** `LVL_Sluz` (временно)
- **Game Default Map:** `LVL_Sluz` (временно)

После Phase 1 D4 dressing — вернуть на нужный default.

### 10.2 Запуск

1. Открыть `LVL_Sluz` (если не открыт): Content Browser → Content/Levels/ → double-click.
2. **Play** (Alt+P, или Toolbar → Play button).
3. Кликни в viewport чтобы захватить курсор.

### 10.3 Чек-лист

- [ ] **Spawn в шлюзе.** Видишь дверь А впереди (placeholder Box на юге, Y=+250). Если смотришь в стену / потолок — проблема с PlayerStart Yaw / Z, см. §6.
- [ ] **WASD** двигает игрока по полу комнаты.
- [ ] **Collision стен** работает — нельзя пройти через стены. Проверь все 4 стены.
- [ ] **Spot light** освещает комнату с потолка. Видишь light cone, освещённость пола, мягкие тени от placeholder boxes.
- [ ] **Sky Light fill** — углы у пола не absolute black (есть legible detail).
- [ ] **PostProcess:** контрастность норм. Не overexposed (всё бело), не black. EV100=10 даёт interior-readable пейзаж. Если over/under — крути EV100 ±2.
- [ ] **Vignette** видна по краям экрана (мягкая, ~35%).
- [ ] **Film grain** subtle на static-cadre — заметно если присматриваться.
- [ ] **Locker / стол / журнал / регламент** видны как placeholder boxes / planes.
- [ ] **Дверной проём** в южной стене проходим — collision корректный (nothing blocks), Box-placeholder двери не блокирует.
- [ ] **FOV ≈ 75°** (визуальная сверка против reference scene с FOV 90 — экран чувствуется уже).
- [ ] **Walk speed ~240 cm/s** — пройти от северной стены (Y=-200) до южной двери (Y=+250) = 450 cm. При 240 cm/s = ~1.9 секунды walk. Чуть дольше с inertia → ~2.5 sec. Если 1 сек или 5 сек — speed broken.
- [ ] **Output Log** (Window → Output Log): нет красных errors. Warnings про Lumen recapture / Sky Light recapture — ок.
- [ ] **Audio reverb** (если slap-test есть звук — выстрел footstep'а): tail ~0.35s, "small room" feel. Если footstep samples ещё не имплементированы (Phase 2) — пропусти этот пункт.

### 10.4 Если что-то не так

| Симптом | Diagnosis |
|---|---|
| Spawn в потолке / падает сквозь пол | PlayerStart Z неверный — adjust до 90 или 88 |
| Spawn смотрит в стену | PlayerStart Yaw неверный — должен быть 90 |
| Всё чёрное | Sky Light Intensity 0, или Spot Light off, или EV100 слишком низкий |
| Всё белое | EV100 слишком высокий, или Spot Light Intensity bonkers |
| Сквозь стены проходит | Box collision не auto-set — выделить box → Details → Collision → Collision Presets = `BlockAll` (default static mesh) |
| Дверной проём заблокирован | Placeholder двери не NoCollision — выдели → Collision Presets = `NoCollision` |
| Нет reverb tail | Audio Volume не покрывает PlayerStart, или Reverb Effect не assigned, или Volume = 0 |

---

## 11. Optional — ANoiseFluorescentLight C++ класс для flicker (TODO Phase 1 D4)

Per layout §2.1.e — лампа мигает раз в 45-90 сек. Это **не делаем в Phase 1 D3** — whitebox без flicker допустим. Обозначаем как будущую задачу.

**TODO для D4:**
- Создать `Source/noise/Lighting/NoiseFluorescentLight.h/.cpp` extends `AActor`.
- Components: `USpotLightComponent` + optional `UStaticMeshComponent` (fixture body).
- UPROPERTY tunable per atmosphere_plan §3 fluorescent flicker spec:
  - `FlickerFrequencyHz = 0.143` (cycle/7s)
  - `FlickerJitterPercent = 30`
  - `DarkFrameDuration = 0.05s`
  - `FlickerAmplitudeMin = 0.85`, `FlickerAmplitudeMax = 1.0`
- Tick: timer-based, randomize next flicker time, pulse intensity DOWN to 0.85 for 0.05s, return to 1.0.
- BP child `BP_FluorescentLight` для placement в level'е.
- **Per layout §2.1.e:** в Шлюзе flicker **раз в 45-90 сек**, не каждые 7 сек как atmosphere_plan §3 default. Шлюз = relatively bright "ещё нормально" зона. Override `FlickerFrequencyHz` на ~0.015 (= 1/65 sec average) на этом instance'е.

`// TODO D4: имплементировать ANoiseFluorescentLight, заменить static SpotLight на BP_FluorescentLight в LVL_Sluz`

---

## 12. Commit

После того как `LVL_Sluz` работает:

1. **UE5 Editor:** File → **Save All**.
2. Закрыть Editor.
3. Проверить changes (GitHub Desktop или CLI):
   ```
   git status
   ```
   Ожидаем:
   - `Content/Levels/LVL_Sluz.umap` (новый)
   - `Content/Audio/Reverb/SoundReverb_Sluz.uasset` (новый)
   - `Content/__ExternalActors__/Levels/LVL_Sluz/...` (если World Partition включён — auto-generated external actors per actor)
   - `Content/__ExternalObjects__/Levels/LVL_Sluz/...` (similarly)
   - `Config/DefaultEngine.ini` (если менял Editor Startup Map / Game Default Map)
4. **НЕ** `git add -A` — может зацепить `/Saved/`, `/Intermediate/`, `/DerivedDataCache/`. Проверь `.gitignore` покрывает их (должен после Phase 1 D1).
5. Specific add:
   ```
   git add Content/Levels/LVL_Sluz.umap Content/Audio/Reverb/SoundReverb_Sluz.uasset Content/__ExternalActors__/Levels/LVL_Sluz Content/__ExternalObjects__/Levels/LVL_Sluz Config/DefaultEngine.ini
   ```
6. Commit:
   ```
   git commit -m "phase1 d3: LVL_Sluz whitebox + lighting + post-process volume"
   ```
7. Push.

---

## 13. Что дальше (Day 4+)

- **D4** — заменить placeholder boxes на реальные меши Cold War Bunker (стены modular wall sections, потолок, door, fluorescent fixture). `SM_Wall_Modular_*` / `SM_Ceiling_*` / `SM_Door_*` / `SM_FluorescentFixture_*` в `Content/ColdWarBunker/Meshes/`.
- **D4** — имплементировать `ANoiseFluorescentLight` + `BP_FluorescentLight`, заменить static SpotLight (см. §11).
- **D5** — Sketchfab manual hunt: Soviet metal desk + double locker + laminated A4 frame (per layout §2.1.g).
- **D5** — Audio Layer 1: 60 Hz hum loop ambient (per atmosphere_plan §2.1) — добавить как Ambient Sound Actor в LVL_Sluz, attenuation flat 2D.
- **D6+** — следующие levels: `LVL_Korridor` (3×18×3м), `LVL_SektsiyaV` (L-shape ~7×10×3.5м), `LVL_ControlRoom` (9×12×3.5м).

---

## 14. Known gotchas / decisions log

### G1 — BSP vs Modeling Tools

Оба работают для whitebox 4×5×3. Modeling Tools предпочтительнее long-term: можно конвертить mesh в `StaticMesh` asset и далее текстурить как обычный prop. BSP — быстрее для одноразовых тестов, но deprecated path в UE5.5+. **Рекомендую Modeling Tools.**

### G2 — Door cut: Boolean Subtract vs три pieces

Boolean Subtract single mesh — чище long-term. Но Modeling Tools sometimes flaky на overlapping geometry — если subtract не получается с первого раза, fallback на три pieces (§5 Опция Б). Visual identical.

### G3 — Spot light Mobility = Stationary

Stationary — Lumen-friendly mode с static shadows + dynamic intensity. Это позволит flicker animation в D4 без полного rebuild lighting. **Movable** даёт full dynamic, но дороже на perf — не нужно для одной лампы. **Static** — нет dynamic intensity → flicker невозможен.

### G4 — Player Start Yaw=90 convention

UE5: Yaw 0 = +X axis (east). Yaw 90 = +Y axis (south). Yaw 180 = -X (west). Yaw 270 = -Y (north). Проверь координат-направление в Editor Viewport gizmo (3D widget при выделении actor'а — стрелки X/Y/Z).

### G5 — EV100 = 10 placeholder

Atmosphere_plan §3 предполагает interior бункер ~ EV100 10-12. 10 = чуть темнее (более горрор), 12 = чуть светлее. **Calibrate playtest'ом D5-6.** Если на playtest всё кажется too dark even в брайт зоне Шлюза → bump до 11-12.

### G6 — PostProcess Vignette 0.35 статичный

Per atmosphere_plan §3 — "статичная виньетка усиливает туннельное зрение без нарастания". Не ramp'ится по timeline (Phase 3 не трогает Vignette). Если на тесте 0.35 кажется too heavy → 0.25.

### G7 — World Partition vs single-level

UE5.5+ default для new map'ов — World Partition ON. Это добавляет `__ExternalActors__/` и `__ExternalObjects__/` подпапки в Content. Не пугаться при `git status` — всё это commit'абельно. Если хочется отключить (less file noise) — World Settings → World Partition Setup → Enable Streaming = False (Phase 1 single map не нуждается в streaming).

### G8 — Box pivot convention

UE5.5 Modeling Tools Box default pivot = bottom center (Z=0 — низ mesh'а). UE5.7 — verify в редакторе. Если pivot center, скорректируй Location.Z (Floor Z=5 вместо 0, Ceiling Z=295 вместо 300, walls Z=150 как в таблице — уже center-compatible).

---

*Spec v1.0 — Phase 1 Day 3. Single source of truth на geometry/asset/lighting/reverb — [`docs/levels/cvet-shum_layout.md §2.1`](../levels/cvet-shum_layout.md) + [`docs/feel/atmosphere_plan.md §3 + §4`](../feel/atmosphere_plan.md).*
