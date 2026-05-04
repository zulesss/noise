# PLAN — ЦВЕТ-ШУМ (codename: noise)

**Status:** DRAFT 2026-05-04 — design package locked, impl not yet started.
**Engine:** Unreal Engine 5.5+
**Repo:** /home/azukki/cvet-shum
**Project codename for filenames:** `noise`
**In-game / docs display name:** `ЦВЕТ-ШУМ`
**Length budget:** 14 дней (3 фазы × ~5 дней).
**Cost:** $0.

---

## Identity

First-person atmospheric horror-puzzle. Советская АЭС, 26 апреля 1986, ночная смена. Игрок — оперативник, выполняющий процедуру проверки. Ненасильственный horror — нет combat, нет монстров, нет скримеров. Ужас через нарастающую неправильность институционального процесса. 25-35 минут одно прохождение, 2 концовки. Русский язык.

**Core verb:** заполнение checklist'а проверки + наблюдение.
**Core fantasy:** «инструкции перестают совпадать с реальностью».

## MVP — критерии "done"

1. Игрок проходит полный checklist (10-12 пунктов) через 4 связанных помещения.
2. ≥3 scripted anomaly-моментa отрабатывают надёжно (manometer drift, journal entry by you, telephone never answers).
3. Финальный пульт даёт 2 разные концовки на основе выбора в журнале.
4. Audio stack полный: 60Hz гул + флуоресцент buzz + footsteps + Silero TTS-голос диктора (русский).
5. Build стабильно запускается на Windows из main.exe без editor.

## Stack

| Слой | Инструмент | Стоимость |
|---|---|:-:|
| Engine | UE5.5+ (Lumen + Nanite) | Free |
| Movement | Stock UE5 `Character` + custom `BP_Player`, FOV 75 lock, walk 240 cm/s | Free |
| Owned: все 4 помещения (стены / потолки / двери) | Cold War Underground Bunker (152 mesh) | ✅ owned |
| Free anchor: industrial detail (трубы, кабели, декаль, мебель) | Derelict Corridor Megascans Sample (262 assets) | Free |
| Materials | PolyHaven + AmbientCG (CC0) | Free |
| Soviet штучные props | Sketchfab CC-BY (5-8 моделей, manual hunt) | Free + cleanup |
| VFX | Niagara Examples Pack UE5.7 | Free |
| Voice (Russian narrator) | Silero TTS v5 (MIT) | Free |
| SFX | Sonniss GDC + Zapsplat + BBC SFX (non-commercial) + Freesound CC0 | Free |
| **Cost total** | | **$0** |

---

## 14-дневный roadmap — 3 фазы

### Phase 1 — Foundation (дни 1-4)

**Цель D4 milestone:** ходишь по 4 помещениям, видишь wall-mounted регламент, поднимаешь журнал, interaction system отвечает.

- UE5 project setup + folder structure (см. context.md) + Git LFS verification для .uasset/.umap
- First-person controller на стоковом UE5 `Character` — `BP_Player` extends `Character` (slow walk 240 cm/s, FOV 75 lock, head bob с attenuation при стоянии, mouse sens default + slider). GASP не используем — see ADR в `WINDOWS_SETUP.md §2`.
- Импорт stack: паки оставляем в корне `Content/` под оригинальным именем (Cold War Bunker → `Content/ColdWarBunker/`, Derelict → `Content/Megascans/`, Niagara → `Content/NiagaraExamples/`) — не двигать через Move, иначе ломаем refs. См. ADR в `WINDOWS_SETUP.md §3`.
- Whitebox layout 4 помещений по docs/levels/cvet-shum_layout.md (Шлюз 5×5×3, Коридор 3×18×3, Секция В L-shape ~10×8×3.2, Control Room 9×9×3.5)
- Lumen + Exponential Height Fog + Volumetric Fog базовый setup
- Флуоресцентное освещение (4000K cool white, 7-сек period flicker через Blueprint timeline на PointLight'ах)
- Interaction system core: trace + outline (subtle геометрический, как Firewatch) + E-press / hold-E (1.5s commit)

### Phase 2 — Core loop (дни 5-9)

**Цель D9 milestone:** полный playable run от шлюза до финального пульта без полировки.

- Checklist UI — paper journal в руках, открывается на Tab, UMG widget с ручной анимацией страниц
- Procedure system — data-driven (DT_Procedures с пунктами + state machine)
- 3 scripted anomaly events (см. docs/narrative/anomaly_choreography.md):
  - **MANOMETER_DRIFT** — manometer 2.4→2.1 при втором посещении коридора
  - **TELEPHONE_NEVER_ANSWERS** — длинные гудки 60+ секунд (425Hz советский тон)
  - **JOURNAL_ENTRY_BY_YOU** — запись с сегодняшней датой, твоим именем, твоим почерком
- Sound foundation Layer 1 (ambient): 60Hz drone loop + флуоресцент buzz + ventilation hum (см. docs/feel/atmosphere_plan.md)
- Silero TTS Russian — Python script для генерации announcement-голоса диктора с обрывом на полуслове (см. voice script в docs/narrative/)
- Vertical slice playthrough end-to-end

**🚨 Critical feel-engineer recommendation:** после имплементации Layer 1 ambient — посадить тестера в комнату с наушниками на 3 минуты. Если "тихо, ничего не происходит" — ambient работает. Если "странно, что-то гудит" — убавить громкость. Весь дальнейший дизайн строится поверх незамечаемого основания.

### Phase 3 — Polish + finale (дни 10-14)

**Цель D14:** ship.

- Final control panel encounter — индикаторы (зелёные vs красные count change между первым проходом и финалом), выбор в журнале, 2 endings
- Visual polish ramp по timeline:
  - **Chromatic aberration:** 0% → 1% к минуте 5 → 5% к минуте 30 (linear ramp)
  - **Film grain:** базовый low → к финалу как старая советская съёмка
  - **Saturation:** 100% → 60% к финалу
  - **Temperature shift:** на минутах 33-34 (момент финального выбора) — единственный раз cool→warm orange
- Audio mix — все 4 layer'а:
  - Layer 1 ambient (60Hz, fluorescent, ventilation)
  - Layer 2 reactive (footsteps, paper, pen, door)
  - Layer 3 critical scripted (phone tone, piano note, sirena)
  - Layer 4 spatial / reverb zones per room
  - Финальная фортепианная нота (одна нота, не аккорд) на финал
  - Концовка 1 — full silence титры
  - Концовка 2 — sirena → tишина → текст отчёта
  - **3.2-секундная пропажа 60Hz hum один раз на минутах 20-30** (главное аудио событие — отсутствие звука который игрок перестал замечать)
- Sketchfab manual hunt — добавить штучные советские props (manometer dial detailed, soviet rotary phone, control panels, laminated регламенты A4) — см. shopping list в docs/levels/
- Playtest 2-3 раза full run, fine-tune timing/pacing
- Windows packaged build

---

## Parallel streams (worktree workflow)

Через `Agent({isolation: "worktree"})` — Claude автоматически создаёт worktree per agent. Три stream'а независимы по файлам:

- **Stream A — Level / Environment:** layout, lighting, asset integration, props placement, Sketchfab hunt + Blender pipeline. Touches `/Content/Levels/`, `/Content/Environment/`. Phase 1 основное, Phase 3 polish.
- **Stream B — Mechanics / Scripting:** interaction, checklist UMG, procedure system, scripted anomalies, finale logic. Touches `/Content/Blueprints/`, `/Content/UI/`, `/Content/Data/`. Phase 2 целиком + finale в Phase 3.
- **Stream C — Audio / Narrative:** Silero TTS pipeline, ambient stack, voice generation, mix bus, post-process ramp. Touches `/Content/Audio/`, `/Content/PostProcess/`. Phase 1 старт (Layer 1 раннее), основной объём Phase 3.

---

## Top 3 risks

1. **UE5 onboarding** — нет глубокого опыта на UE5. Заложен 1-2 day buffer в Phase 1. Mitigation: стоковый First Person Template + UE5 docs прежде чем строить custom; Blueprint default (не C++) на прото.
2. **Atmospheric pacing** — 35-минутная игра без combat'а легко становится "медленно и скучно". Mitigation: D9 milestone — full playthrough есть, Phase 3 имеет 5 дней на pacing tuning.
3. **Audio half the game** — если 60Hz гул и Silero голос звучат любительски, нарратив рушится. **Mitigation: feel-engineer рекомендация — Layer 1 ambient тестировать в наушниках 3 минуты до построения любого upper-layer.**

---

## Resolved design decisions (locked 2026-05-04)

### Q1 — Финальный пульт: **без фиксации чисел в журнале** ✓
Игрок впишет 14З/2К как свободный текст пункта 4.1, но никакого system-level cross-check'а с финальным состоянием пульта. Paranoia route — игрок сомневается, видел ли он 14 или 11 зелёных. Эмоция финала: сомнение, не клиническая уверенность.

### Q2 — Дверь Секция В → Control Room: **always open** ✓
Никаких trigger-conditions. Все двери проходимы в обе стороны на всём протяжении игры, кроме финального lock'а двери В после взаимодействия с финальным пультом.

### Q3 — Walk speed: **240 cm/s lock на Day 1** ✓ (recalibrate D5-6)
Conservative ставка. Если ощущается тягомотиной на playtest D5-6 → поднять до 300. Не до 480. Shift = 320 cm/s.

### Q4 — 65-сек telephone UX cue: **текст пункта 3.2** ✓
Прямая инструкция в пункте регламента: "не вешать трубку до ответа диспетчера". Никаких UI-prompt'ов, toast'ов, indicator'ов.

### Q5 — CLOCK_WALKS_BACK: **оставить тихой** ✓
Опциональная anomaly без процедурного крючка. Часть anomaly остаются на periphery — это часть дизайна. Не каждый игрок должен заметить всё.

### UE5 setup
**Анриал на Windows у пользователя.** Linux dev box — без UE5 binary. Workflow: text/code/config/git делается на Linux, .uasset/.umap/импорты ассетов/playtest — на Windows-стороне. Headless validation на Linux недоступна — `CompileAllBlueprints` запускается пользователем на Windows вручную при необходимости.

---

## Design artifacts

- [docs/narrative/anomaly_choreography.md](docs/narrative/anomaly_choreography.md) — checklist (10-12 пунктов), anomaly events (5-7), voice script, finale 2 endings, tension curve
- [docs/levels/cvet-shum_layout.md](docs/levels/cvet-shum_layout.md) — 4-room spatial layout, asset usage map, key feature placement, lighting zones
- [docs/feel/atmosphere_plan.md](docs/feel/atmosphere_plan.md) — sound layered stack, post-process timeline, camera/movement spec, TTS pipeline

---

## First impl delegation — что делать в новой сессии

После открытия новой Claude Code сессии в `/home/azukki/cvet-shum/`:

1. Read `.claude/context.md` + `PLAN.md` + 3 design docs (`docs/narrative/`, `docs/levels/`, `docs/feel/`)
2. Day 1 Linux-side (text-only через `unreal-engineer` agent):
   - `noise.uproject` (EngineAssociation 5.5)
   - `Config/DefaultEngine.ini` — Lumen + Volumetric Fog defaults
   - `Config/DefaultGame.ini` — project ID, default map placeholder
   - `Config/DefaultInput.ini` — WASD + mouse + E + Shift + Tab, mouse smoothing OFF
   - Folder structure (`Content/Audio/...`, `Content/Blueprints/...`, и т.д.) с `.gitkeep`
   - `WINDOWS_SETUP.md` — checklist для пользователя на Windows-стороне
3. Day 1 Windows-side (пользователь, GUI-based):
   - Открыть `noise.uproject` в UE5 Editor 5.5+
   - Создать пустой `LVL_Empty.umap` в `Content/Levels/`
   - Импортировать packs (Cold War Bunker, Derelict Corridor, Niagara Examples Pack) — Soviet/Post Soviet World отвалился (см. ADR в `WINDOWS_SETUP.md §3` + `.claude/context.md`), функции покрыты Bunker + Sketchfab + Material decals
   - Fix Up Redirectors на корневой `Content/`
   - Push изменения обратно в git
4. Day 2+ — `BP_Player` extends stock UE5 `Character` (FOV 75 lock, walk 240, head bob spec из atmosphere_plan) — Blueprint, делается на Windows-стороне с briefing через chat. GASP не используем (overkill для first-person slow-walk horror — third-person motion matching showcase).
