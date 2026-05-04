# WINDOWS_SETUP — ЦВЕТ-ШУМ (codename `noise`)

Этот документ — checklist Windows-стороны после `git pull` свежего скелета. Linux dev box собирает text/config/git, UE5 Editor (только Windows у пользователя) делает .uasset/.umap/импорты.

---

## 0. Pre-requisites

- Windows 10/11
- Git for Windows + Git LFS (`git lfs install`) — обязателен, иначе `.uasset`/`.umap`/`.fbx`/`.wav` приедут как stub-pointers
- Epic Games Launcher
- Unreal Engine 5.5+ (5.6 / 5.7 ок, тогда после первого open подменить `EngineAssociation` в `noise.uproject` через ПКМ → "Switch Unreal Engine version" — UE5 пропишет автоматически)
- Свободно ~100 GB на диск (engine + project cache + paid packs)

---

## 1. Первый open проекта

1. `cd` в `cvet-shum/` локально на Windows.
2. `git lfs pull` (один раз — подтянуть LFS-объекты, если на этом этапе уже были).
3. ПКМ на `noise.uproject` → **"Switch Unreal Engine version"** → выбрать установленную 5.5+. (Если версия только одна — UE5 автоматически прицепится при двойном клике.)
4. Двойной клик `noise.uproject`.
5. UE5 предложит сгенерировать недостающие `.uplugin`/Source-модули. Согласиться (никаких C++ модулей у нас нет — Blueprint-only, эта диалоговая ветка просто не сработает / закроется быстро).
6. После загрузки UE5 покажет пустой Editor. Output Log проверить на красные ошибки. **Желтые warnings про missing assets** — нормально на этом этапе (мы ещё не импортировали paid packs).
7. **Создать пустой level:**
   - File → New Level → "Empty Level" → Save As → `Content/Levels/LVL_Empty.umap`
   - Это нужно потому что `Config/DefaultGame.ini` ссылается на `/Game/Levels/LVL_Empty.LVL_Empty` как default. Без файла UE5 откроет fallback-empty но запишет warning.

---

## 2. Movement / Character — НЕ GASP

**ADR (2026-05-04):** GASP (Game Animation Sample Project) не устанавливаем.

GASP — это showcase third-person motion matching от Epic (Manny бегает / прыгает / climbs / vaults). У нас first-person slow-walk horror без traversal, без combat, без crouch/jump/sprint. От GASP остаётся ~0% полезной поверхности — тащить ~5 GB ассетов в `Content/` чтобы не использовать ничего смысла нет.

**Что делаем:** `BP_Player` extends стоковый UE5 `Character` с нуля. Спецификация из `docs/feel/atmosphere_plan.md §1`:

- `CharacterMovementComponent.MaxWalkSpeed = 240` (normal walk)
- Shift hold → `MaxWalkSpeed = 320` (cautious jog, без отдельной анимации, без stamina)
- `FirstPersonCamera` (CameraComponent attached к capsule, FOV 75 lock)
- Head bob: sin-wave на CameraComponent.RelativeLocation (Y/X amplitudes 0.8/0.4 cm, freq 1.2/1.6 Hz, exp decay tau 0.4s — полная статика при velocity < 5 cm/s)
- Pitch limits: +75° / -60°
- Mouse sensitivity 0.18 default, smoothing OFF, accel OFF
- No crouch, no jump, no sprint FOV kick

Day 2 task: брифовать `unreal-engineer` на структурированную BP-спецификацию для Editor'а. ~150-200 нод, 1 день работы.

---

## 3. Импорт paid / free packs

**ADR (2026-05-04):** паки оставляем в **корне `Content/` под оригинальным именем папки**, не двигаем через Move в `Content/Environment/<имя>/`. UE5 хранит hardcoded `/Game/<original>/...` пути внутри `.uasset` — каждый Move риск broken refs. На 14-day прототипе простота важнее folder hierarchy. Свою authored работу (Audio, Blueprints, Levels, Materials, Sketchfab hunt props) — в наших организованных папках.

### Сценарии импорта

**3a. "Add to Project" из Fab (in-editor):**

1. UE5 Editor → **Window → Get Content** (Fab plugin внутри редактора)
2. Залогиниться Epic-аккаунтом
3. Library tab → найти pack → **Add to Project** → выбрать `noise`
4. Pack окажется в `Content/<своя подпапка>/` — **оставить как есть, не двигать**
5. **Fix Up Redirectors in Folder** на корневом `Content/` (на всякий случай)

**3b. Migrate из standalone `.uproject`:**

1. Открыть исходный проект в UE5, дождаться компиляции шейдеров
2. Content Browser → ПКМ на **`/Game` root** (или на корневой папке pack'а) → **Asset Actions → Migrate**
3. Destination → `<noise>/Content/` → **Select Folder** (UE5 сам вложит относительно `/Game/`)
4. Дождаться "Content migrated successfully", закрыть исходный проект
5. Открыть `noise.uproject` — pack окажется в `Content/<original folder>/` — **оставить как есть, не двигать**

**3c. Pack как голая Content-папка (без `.uproject`):**

Типичный кейс когда pack распространяется как просто папка с `.uasset`/`.umap` (zip):

1. **Закрыть UE5 полностью** (не просто проект — весь Editor)
2. Windows Explorer: скопировать содержимое pack-папки **с сохранением структуры** в `<noise>/Content/` так чтобы корневое имя совпадало с тем что внутри `.uasset` reference
   - Например: pack лежит как `D:\packs\ColdWarBunker\Content\ColdWarBunker\<subfolders>` → копируем `ColdWarBunker\<subfolders>` целиком в `<noise>\Content\ColdWarBunker\`
3. Открыть `noise.uproject` — UE5 проиндексирует
4. **Если меши pink / материалы битые** — copy structure не совпала с тем что хранится в reference:
   - Открой один pink material → **Reference Viewer** (`Alt+Shift+R`)
   - Reference Viewer покажет ожидаемый path (например `/Game/ColdWarBunker/Materials/M_Concrete`)
   - Подгонишь иерархию на диске под него и переоткрой проект

### Pack-specific notes

- **Cold War Underground Bunker** (https://www.fab.com/listings/5a4b7b4c-67cd-47ff-bdd5-6373e7b62dc6) — типичный путь после миграции `Content/ColdWarBunker/`. Покрывает все 4 помещения (стены, потолки, двери, fixtures).
- **Derelict Corridor Megascans Sample** (https://www.fab.com/listings/d825ff9c-77da-45a4-9619-c989e6dfdda6, free) — типичный путь `Content/Megascans/<имя>/` или `Content/DerelictCorridor/`. 262 ассета + 11 environment materials. Распространяется как standalone project — workflow 3b.
- **Niagara Examples Pack UE5.7** (https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600, free) — типичный путь `Content/NiagaraExamples/`. **Требует engine 5.7** — если у тебя 5.5, bump `EngineAssociation` через ПКМ на `.uproject` → "Switch Unreal Engine version".

**ADR (2026-05-04):** Soviet/Post Soviet Abandoned World pack отвалился из Fab Library пользователя. Заменён на: Cold War Bunker (двери, базовые модули) + Sketchfab CC-BY (locker / operator desk / chair / штучные props) + Material decals от PolyHaven CC0 / AmbientCG (peeling paint, warning signs).

### Подводные камни

- **Розовые / чекерные материалы** после migrate — зависимости не подтянулись. Open the Material → **Reference Viewer** покажет ожидаемый path missing texture. Решение: вернуться в исходный проект, мигрировать тот конкретный ассет ещё раз поверх. Если pack без проекта — поправить структуру на диске под ожидаемый path.
- **"Material reset" на меше** (slot пустой) — меш ссылается на материал по path который не существует в текущем проекте. Чаще всего после Move'а папок в неправильном порядке. Фикс: положить материал по тому пути который меш ожидает (Reference Viewer покажет).
- **Megascans Bridge plugin** — некоторые Megascans материалы зависят от Bridge plugin'а. Если warnings — добавить `Megascans` (или `Bridge`) в `Plugins` секцию `noise.uproject`.
- **LFS push будет тяжёлый** — текстуры 4K-8K, ожидай 1-3 ГБ через LFS. Не отменять `git push` посредине.
- **Никаких rename/move через Windows Explorer / git mv** для УЖЕ работающих pack'ов — только через UE5 Editor + Fix Up Redirectors (но мы и так не двигаем — packs at root).

---

## 4. Plugins после первого open

`noise.uproject` уже включает:

- `Niagara` (default ON в UE5 — оставлен явно)
- `EnhancedInput` (UE5.5+ default; legacy mappings в `DefaultInput.ini` — fallback пока IMC_Default не создан в Editor'е)
- `ModelingToolsEditorMode` (для whitebox layout — Phase 1)

При первом open UE5 может показать "missing plugin" если у тебя версия engine'а < 5.5 — апгрейдни engine.

---

## 5. Decisions / known issues

### `ProjectName=CVET-SHUM` (ASCII), не "ЦВЕТ-ШУМ"

В `Config/DefaultGame.ini` ключ `ProjectName=` принимает только ASCII (UE5 кладёт это значение в build paths и executable name — Unicode там ломает packaging на Windows). Display-имя для UI/launcher хранится в `ProjectDisplayedTitle=NSLOCTEXT(...)` с локализуемым "ЦВЕТ-ШУМ" — это видит игрок в окне Editor и в packaged game. Description в `noise.uproject` тоже на латинице/UTF-8 mix — там Unicode ОК.

Если UE5 5.5 на open всё-таки ругнётся на кириллицу в `ProjectDisplayedTitle` — поправим: вынесем в локализационный `.archive` файл. На 5.5 stable должно работать.

### Legacy vs Enhanced Input

Я записал legacy `+AxisMappings`/`+ActionMappings` в `DefaultInput.ini` — они работают сразу из коробки и BP_Player сможет связаться с ними через Input nodes. Это **временная мера** на Phase 1. На Phase 2:

1. В Editor'е создать Input Action assets:
   - `IA_Move` (Axis2D, Vector2D)
   - `IA_Look` (Axis2D, Vector2D)
   - `IA_Interact` (Boolean)
   - `IA_CautiousJog` (Boolean, hold)
   - `IA_OpenJournal` (Boolean)
2. Создать Input Mapping Context `IMC_Default` в `Content/Input/` — забиндить WASD / Mouse / E / Shift / Tab.
3. В `BP_Player` BeginPlay: получить `EnhancedInputLocalPlayerSubsystem` → AddMappingContext (IMC_Default, priority 0).
4. Заменить вызовы Input в BP_Player на Enhanced Input nodes.
5. Удалить legacy mappings из `DefaultInput.ini`.

### Niagara Examples Pack v5.7 на UE5.5

Если engine 5.5 ругнётся на 5.7 pack — bump engine до 5.6/5.7 через `EngineAssociation` или мигрируй вручную нужные systems из standalone-проекта.

### Audio sample rate 48 kHz

`DefaultEngine.ini` пинит Audio Mixer на 48000 Hz / 1024 callback frame size. Это сходится с TTS/SFX pipeline в `docs/feel/atmosphere_plan.md`. Если плейтест выявит latency — снизим до 512.

### Volumetric Fog default ON, но per-scene density на актёре

`r.VolumetricFog=1` глобально. Конкретные значения (Density 0.015, Albedo 0.9/0.92/0.95, Extinction 1.2 — atmosphere_plan §3) выставляются на `ExponentialHeightFog` actor'е каждого level'а. На пустом `LVL_Empty` — никакого fog'а нет, это OK. Реальные level'а (`LVL_Sluz`/`LVL_Korridor`/...) будут получать fog при создании в Phase 1.

### Auto Exposure фиксированный

Глобальный CVar `r.AutoExposure.MethodOverride=-1` (no override). Реальный fix — через PostProcessVolume на каждом level'е: `Exposure → Metering Mode = Manual`, EV100 ~10-12 для тёмного бункера. Обязательно в Phase 1 lighting setup.

---

## 6. Git protocol после Windows-side изменений

После того как импортнул paid packs / создал `LVL_Empty.umap` / собрал first BP_Player:

1. UE5 → ПКМ на `Content/` → **Fix Up Redirectors in Folder** (обязательно перед каждым commit'ом если что-то rename/move'ил).
2. Закрыть Editor (некоторые .uasset не сохраняются на диск пока редактор открыт).
3. `git status` — убедиться что нет `Saved/` `Intermediate/` `DerivedDataCache/` (они в `.gitignore`).
4. `git add Content/<конкретные новые .uasset> Config/<обновлённые .ini> noise.uproject`. **НЕ** `git add -A`.
5. `git commit -m "phase1 d1: <что сделал>"` (Co-Authored-By не нужен — это твоя работа).
6. `git push`.

Опциональная sanity-проверка локально на Windows (если UE5 в PATH):

```cmd
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
  "%CD%\noise.uproject" -run=CompileAllBlueprints -unattended
```

Exit 0 = ок. Любые ошибки — diagnose до push'а.

---

## 7. Что НЕ делать

- Не коммитить `Saved/`, `Intermediate/`, `Binaries/`, `DerivedDataCache/`, `*.pdb` (всё в `.gitignore`).
- Не rename/move `.uasset` через Windows Explorer / git mv — только через UE5 Editor + Fix Up Redirectors. Иначе Blueprint references break.
- Не запекать Lumen → Lightmass без явной причины — у нас Lumen real-time GI by design.
- Не включать Nanite на skeletal meshes и translucent materials — UE5.5 не поддерживает.
- Не апгрейдить engine на 5.6/5.7 в середине Phase 1 без cause — добавит buffer time на migration.

---

*Версия 1.0 — 2026-05-04. Обновляется по факту разрешения Windows-side issues.*
