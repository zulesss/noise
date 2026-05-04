# Player Implementation Spec (UE5.5+) — C++ base + thin BP child

**Version:** 2.0 (2026-05-04). v1.0 был pure-Blueprint спекой на 200+ нод; переписан после решения уйти в C++.

**Reference (single source of truth):** [`docs/feel/atmosphere_plan.md §1`](../feel/atmosphere_plan.md) — все feel-параметры (FOV, walk speed, head bob, mouse, pitch limits) берутся оттуда. Если число расходится — atmosphere_plan прав.

---

## 1. Цель и workflow

`ANoiseCharacter` (C++, `Source/noise/Player/`) владеет всей логикой первого лица: components, input bindings, head bob, pitch clamp. `BP_Player` — тонкий Blueprint child для:

- Per-instance tweaks defaults через Class Defaults (если хочется чуть другую sensitivity на конкретном level'е).
- Designer-friendly attach point для будущих animation hookup'ов / SFX / VFX компонентов.
- Construction Script polish если позже понадобится.

Архитектурный сдвиг **с pure-BP на C++ + thin BP child** сделан потому что 200+ нод graph collapse'ом сложнее ассемблить и поддерживать чем ~200 строк читаемого C++. Designer-friendly defaults остаются — все параметры — `UPROPERTY(EditAnywhere)`.

**Phase 1 input lock:** legacy `AxisMappings`/`ActionMappings` из `Config/DefaultInput.ini`. Migration на Enhanced Input — Phase 2 (см. §9 TODO).

---

## 2. Windows-side one-time setup

1. Установить **Visual Studio 2022 Community** с workload "Game development with C++". Проверить components: MSVC v143, Windows 10/11 SDK, .NET desktop dev (для UnrealBuildTool).
2. `git pull` — на Windows pull origin/main (Source/ folder + uproject Modules section).
3. Двойной клик `noise.uproject` в Explorer'е — UE5 покажет диалог:
   > "The following modules are missing or built with a different engine version: noise. Would you like to rebuild them now?"

   → **Yes**.
4. UE5 spawn'ит UnrealBuildTool, скомпилит модуль (~30-60 сек на современном железе). Логи — в `Saved/Logs/`.
5. После успеха Editor загрузится. В Pick Parent Class будут доступны `NoiseCharacter` и `NoiseGameModeBase`.

**Если build падает:** прочитай Output → найди MSBuild error → если `error MSB4019: imported project not found` — переустанови VS workloads. Если `cannot find Engine.Build.cs` — удали `Binaries/`, `Intermediate/`, ПКМ на uproject → "Generate Visual Studio project files" → попробуй снова.

---

## 3. Создание BP_Player (Editor)

1. Content Browser → `Content/Blueprints/Player/` (создать папку если нет).
2. ПКМ → **Blueprint Class**.
3. **All Classes** → search `NoiseCharacter` → выбрать.
4. Имя: `BP_Player`. Save.
5. Открыть. Defaults уже унаследованы из C++ конструктора — ничего трогать не обязательно для Phase 1.

Опционально подкрутить в **Class Defaults**:
- `Mouse Sensitivity` → если 0.18 окажется слишком медленно/быстро на playtest D5-6.
- `Walk Speed Normal` → recalibration window per PLAN.md Q3 (240 → возможно 300).

Все feel-параметры сгруппированы в категории `Noise|Movement`, `Noise|Camera`, `Noise|HeadBob`. FirstPersonCamera виден в Components panel, но сам компонент — read-only (создан в C++ конструкторе).

---

## 4. Создание BP_GameMode_Default

1. `Content/Blueprints/` → ПКМ → **Blueprint Class** → All Classes → search `NoiseGameModeBase` → выбрать.
2. Имя: `BP_GameMode_Default`. Save.
3. **Class Defaults** → **Default Pawn Class** → выбрать `BP_Player` (НЕ `NoiseCharacter`).

   *Decision:* C++ конструктор set'ит `DefaultPawnClass = ANoiseCharacter::StaticClass()`, но designer хочет spawn'а **BP_Player** (чтобы любые BP-уровневые tweaks вступали в силу). Override на BP-уровне — стандартный UE5 паттерн. C++ default — fallback если BP_Player не указан.

4. **Player Controller Class** = `PlayerController` (стоковый, не делаем custom для Phase 1).
5. Compile + Save.

---

## 5. Project Settings

**Edit → Project Settings → Maps & Modes:**
- **Default GameMode** = `BP_GameMode_Default`
- **Editor Startup Map** / **Game Default Map** = `LVL_Empty` (или другой test level с PlayerStart)

Per-map override — через World Settings → GameMode Override на конкретном level'е если позже понадобится.

---

## 6. Smoke test PIE

После compile + save + open level с PlayerStart → **Play In Editor**:

- [ ] WASD двигает игрока forward/back/strafe
- [ ] Mouse look yaw + pitch работают
- [ ] Pitch clamped: до упора вверх — стоп на +75°, вниз — на -60°
- [ ] Walk speed @ normal ≈ 240 cm/s (Console: `Show Velocity` или DrawDebugString на Tick: `GetVelocity().Size()`)
- [ ] Shift hold ускоряет до 320 cm/s, без FOV kick / sprint feel
- [ ] Head bob: видим во время движения, decay при остановке (intensity к 0 за ~1.5 sec, hard zero через 0.5s после velocity < 5 cm/s)
- [ ] Mouse smoothing OFF — резкий sync без inertia
- [ ] FOV ровно 75 (визуальная сверка против reference scene при FOV 90)
- [ ] Space — ничего не делает (no jump, JumpMaxCount=0)
- [ ] Tab — `[Noise] Journal toggle` в Output Log
- [ ] E — `[Noise] Interact pressed` в Output Log
- [ ] Output Log: нет warnings/errors с префиксом `LogNoise` или `LogBlueprint`

**Если что-то не так:** Output Log → Filter → Errors. C++ ошибка — починка Source + recompile через editor toolbar **Compile**. BP-уровень — обычная `Compile` кнопка в BP editor.

---

## 7. Workflow для последующих изменений

Два пути для feel-tweaks:

### Quick tweak (designer scope)
- Open `BP_Player` → Class Defaults → подкрутить значение → Compile → Save → PIE.
- Мгновенно, без C++ recompile, без перезапуска editor'а.
- Применимо к: walk speeds, head bob amplitudes/freqs, mouse sensitivity, pitch limits, FOV.

### Architectural change (engineer scope)
- Edit `Source/noise/Player/NoiseCharacter.h` или `.cpp`.
- Push на main (Linux-side) → user pull (Windows) → Editor toolbar **Compile** (Live Coding) или close → rebuild → reopen.
- Применимо к: новые UPROPERTYs, новые input handlers, изменения tick-логики, добавление компонентов.

---

## 8. Known gotchas (UE5.5+)

1. **Live Coding flaky на UCLASS/UPROPERTY changes.** Если добавил новый UPROPERTY и Live Coding зелёный, но Class Defaults не показывает поле — close editor, full rebuild через VS, reopen. Для изменений тел методов Live Coding обычно работает.
2. **VS не открывается автоматически на double-click `.cpp` из Editor?** File → Generate Visual Studio project files в UE5. Если не помогает — установлен ли VS Integration tool?
3. **PIE crash на старте после изменения GameMode** — проверь что `Default Pawn Class` в `BP_GameMode_Default` НЕ None и НЕ удалённый класс.
4. **Mouse не двигает камеру** — проверь что `FirstPersonCamera->bUsePawnControlRotation = true` (set в C++ конструкторе; если override'нул в BP — снова включи).
5. **Head bob не вижу** — `PrimaryActorTick.bCanEverTick = true` set'ed в C++ конструкторе, но если BP_Player override'нул — Tick не зовётся.
6. **Mouse capture в PIE** — кликни в viewport чтобы захватить курсор. Shift+F1 — release. Стандартное поведение.
7. **Hot reload и derived BPs:** после C++ рестарта может потребоваться recompile BP_Player и BP_GameMode_Default (Editor сам предложит).

---

## 9. TODO (next sessions)

- **Phase 2 — Enhanced Input migration.** Создать Input Actions:
  - `IA_Move` (Vector2D) — WASD composite
  - `IA_Look` (Vector2D) — mouse XY
  - `IA_Interact` (Bool) — E
  - `IA_CautiousJog` (Bool) — LeftShift
  - `IA_OpenJournal` (Bool) — Tab

  + `IMC_Default` (Input Mapping Context) с bindings. Удалить legacy mappings из `DefaultInput.ini`. C++ переключить на `UEnhancedInputComponent::BindAction`.

- **Phase 2 — Interaction system.** Отдельный C++ компонент `UNoiseInteractionComponent` (attach к Character), trace из FirstPersonCamera, outline render через CustomDepth + post-process material. См. atmosphere_plan §4.

- **Phase 2 — Journal UI.** UWidget child class `UNoiseJournalWidget`, открывается на Tab. Diegetic — paper в руках, не overlay.

---

## 10. Decisions log

### Decision 1: BP_GameMode_Default override DefaultPawnClass на BP_Player

C++ конструктор ставит `DefaultPawnClass = ANoiseCharacter::StaticClass()`. Но spawn'ить хотим **BP_Player**, не голый C++ класс — иначе любые BP-tweaks игнорируются. Override в Class Defaults BP_GameMode_Default — стандартный UE5 паттерн. C++ default остаётся как safety fallback.

### Decision 2: Phase wraparound через Fmod 2π

`HeadBobPhase += DeltaSeconds * FreqHz * 2π` каждый tick. Через час игры (~27144 rad) float precision начинает портиться. `Fmod` стоит 0 на perf — defensive против 24h playtest sessions.

### Decision 3: Mouse sensitivity в C++ UPROPERTY, scale=1.0/-1.0 в .ini

Scale=1.0/-1.0 в `DefaultInput.ini` (raw delta), множитель `MouseSensitivity` (default 0.18) живёт в C++/BP — runtime tweakable, готов к Settings slider в Phase 3 без правки .ini.

### Decision 4: bIsCautiousJogging boolean (не check MaxWalkSpeed)

Семантика "игрок зажал shift" отделена от "текущая max speed". В Phase 2-3 будут slow zones / debuff'ы — boolean выживает, MaxWalkSpeed-check сломался бы.

### Decision 5: Mesh hidden через SetVisibility(false)

Phase 1 lock: directional light отсутствует (atmosphere_plan §3 — single emissive lighting). Body shadow от owner mesh нерелевантен. SetVisibility(false) проще чем bOwnerNoSee и явный intent. Phase 3 — пересмотреть если решим добавить hands.

### Decision 6: Pitch clamp через PlayerCameraManager.ViewPitchMin/Max в BeginPlay

UE5 применяет клампы автоматом после `AddControllerPitchInput`. Ручной clamp через `Get/Set Control Rotation` дублирует логику. Один set в BeginPlay — достаточно.

### Decision 7: Один C++ класс, без sub-components

Head bob — прямо в `Tick()`, не выделен в `UHeadBobComponent`. Phase 1 простота важнее decoupling. Если в Phase 3 head bob потребует state-machine (breathing, leaning, fall recovery) — рефакторим тогда.

### Decision 8: BP_FirstPersonController НЕ создаём

Стоковый `APlayerController` хватает для Phase 1. Custom controller с camera shake / settings — Phase 3 если понадобится.

---

## 11. Что НЕ делать (Phase 1 scope lock)

- Не добавлять FOV slider — FOV 75 lock per atmosphere_plan §1 (Accessibility checkbox — Phase 3+).
- Не добавлять crouch / sprint / jump / breathing.
- Не делать Enhanced Input migration сейчас.
- Не выделять `UHeadBobComponent` / `UMovementController` — flat класс.
- Не использовать UE5 First Person Template assets как parent (`FirstPersonCharacter`) — там gun mesh / projectile spawn, ненужный шум.

---

## 12. Side-effect: Config/DefaultInput.ini

Per Decision 3 — `LookYaw`/`LookPitch` уже Scale=1.0/-1.0 в `DefaultInput.ini` (применено на Day 1). Sensitivity = `Scale × MouseSensitivity` = `1.0 × 0.18` = 0.18 эффективные. Никаких правок не требуется в этой сессии.

---

*Spec v2.0 — Phase 1 Day 2 revision. Single source of truth на feel-параметры — [`docs/feel/atmosphere_plan.md §1`](../feel/atmosphere_plan.md).*
