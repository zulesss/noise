# ЦВЕТ-ШУМ — Atmosphere Plan

Feel-spec. Версия 1.0. Не финальный контракт — sub-параметры пивотируются после первого hands-on теста.

---

## 1. Camera + Movement

### FOV

**Значение:** 75° — lock. Slider в Settings не предоставляется.

Обоснование: FOV 75 — дизайн-решение, а не accessibility-параметр. Именно компрессия периферии создаёт клаустрофобию. Если давать slider, игроки автоматически тянут к 90-100 и теряют эффект. Единственное исключение — отдельный checkbox "FOV accessibility mode = 90°" для тех, у кого motion sickness на узком FOV. Этот checkbox не рекламировать, держать в разделе Accessibility без описания "зачем".

### Walk speed

UE5 Character Movement компонент, `MaxWalkSpeed`:

| Режим | cm/s | ft/s | Ощущение |
|---|---|---|---|
| Normal walk | 240 | ~7.9 | Осторожная ходьба по коридору |
| Shift hold (cautious jog) | 320 | ~10.5 | Небольшое усилие, не бег |

UE5 default `MaxWalkSpeed` = 600 cm/s. "80% стандарта" = 480 cm/s — это всё ещё быстрее, чем нужно для атмосферы. Рекомендую 240 cm/s как базу: это скорость осторожной ходьбы в плотном пространстве (~2.4 км/ч), не болезненная медлительность. При 480 cm/s игрок пробежит 4 помещения за 2 минуты и атмосфера не успевает осесть.

Shift = `MaxWalkSpeed` 320 cm/s, без отдельной анимации, без stamina bar. Просто чуть быстрее — как человек, который ускорился но не побежал. Никакого sprint FOV kick, никакого breathing.

### Head Bob

```
Amplitude Y (vertical):   0.8 cm
Amplitude X (lateral):    0.4 cm
Frequency:                1.2 Hz при normal walk, 1.6 Hz при Shift
Attenuation:              exponential decay, tau = 0.4s
Full stop threshold:      velocity < 5 cm/s → начинает decay
Dead stop:                полностью нуль через 0.5s после остановки
```

Принцип: head bob должен быть едва заметен. Смотришь прямо — не видишь. Смотришь на дверной косяк сбоку поля зрения — чувствуешь. Это про presence, не про анимационную "живость". Dead Cells-style energetic bob здесь убьёт атмосферу.

Когда игрок стоит — камера абсолютно статична. Это важно: аномалии читаются за счёт микро-перцепции. Если камера постоянно плывёт — игрок не поймёт двигается ли что-то в мире или это боб.

### Mouse Sensitivity

```
Default:   0.18 (при DPI 800, это ~26 cm/360°)
Slider:    0.05 – 0.50
Smoothing: 0 (raw input) — никакого mouse smoothing
Mouse acceleration: OFF
```

Никакого инерционного drift'а. В horror важно точное управление — именно потому что игрок смотрит на каждую деталь.

### Camera Pitch Limits

```
Up:   +75° (не стандартные 85°)
Down: -60° (не стандартные -85°)
```

Срезать немного сверху и снизу. Смотреть строго вниз под ноги не нужно, смотреть строго вверх — потолки не интересны. Незначительное ограничение добавляет subconscious ощущение, что пространство давит. Игрок не заметит ограничения, пока не попробует посмотреть прямо вертикально.

### Crouch

Нет. Не нужен. Помещения не предполагают crawlspace'ов. Добавление crouch создаст expectation что где-то он понадобится — это ложное обещание.

### Footstep Volume + Timing

```
Material:         linoleum (Шлюз, Коридор), concrete (Секция В), metal grate (Секция В — зона у труб)
Step interval:    каждые 0.55s при normal walk, 0.42s при Shift
Volume relative:  -14 dB относительно ambient master (т.е. шаги слышны чётко, но не доминируют)
Stereo spread:    L/R alternation с небольшим (10 мс) delay — реалистично, не игровой "thud-thud"
Reverb send:      шаги посылаются в room reverb с wet 35%
```

Линолеум звучит иначе чем бетон — это важно для пространственной ориентации. Металлическая решётка в Секции В — самый звонкий материал, намеренно: зона у труб ощущается "обнажённо".

---

## 2. Sound Design — Full Layered Stack

### Layer 1 — Persistent Ambient (всегда играет)

| Source | Freq / Char | Volume | Notes |
|---|---|---|---|
| Soviet grid hum | 60 Hz + 120 Hz + 180 Hz | -28 dB | Бесшовный loop 30s+. Нижний край слышимости, воспринимается как тишина пока не выключить. |
| Fluorescent buzz | 800–1400 Hz + гармоники ~2.8 kHz | -34 dB global, -22 dB near fixture | 3D spatial source у каждой лампы. Attenuates по inverse square до дальности 250 cm. |
| HVAC / ventilation | 80–150 Hz, слабый rumble | -32 dB | Flat 2D, не spatial. Ощущается как фоновая вибрация здания. Скорость модуляции 0.07 Hz (медленное дыхание). |

**Требования к loop'ам:** все Layer 1 файлы — минимум 30 секунд, loop-points вне слышимых крестинг/кликов. 60 Hz hum выровнен по фазе на loop point (иначе будет слышен щелчок при цикле). Freesound query: "electrical hum 60hz seamless".

**Spatial setup:** лампы — по одному Ambient Sound Actor на fixture. Radius attenuation: Linear falloff, Falloff Distance = 300 cm, VolumeMultiplier у источника = 0.6. Глобальный 60 Hz hum и HVAC — flat 2D через Master Submix, без позиционирования.

### Layer 2 — Reactive Sounds (действия игрока)

| Action | Sound | Volume | Duration | Notes |
|---|---|---|---|---|
| Footstep linoleum | Soft rubber sole, мягкий шлепок | -14 dB rel ambient | 80–120 ms | 4–6 вариантов рандомизации |
| Footstep concrete | Твёрдая подошва, hollow resonance | -12 dB rel ambient | 100–140 ms | Секция В |
| Footstep metal grate | Металлический clang с tail | -8 dB rel ambient | 150–200 ms | Гриль у труб. Намеренно резкий. |
| Journal page turn | Бумажное шуршание | -6 dB rel ambient | 200–350 ms | ГИПЕРРЕАЛЬНЫЙ — чётче чем всё остальное |
| Pencil scratch | Графит по бумаге | -8 dB rel ambient | 0.5–2.0 s | Loop пока идёт hold-E запись |
| Handle click | Сухой механический щелчок | -4 dB rel ambient | 30–50 ms | При interact-confirm |
| Door open | Советская металлическая дверь: скрип + reverb tail | -10 dB rel ambient | 1.5–2.5 s | Reverb send 60%, wet. Long tail. |
| E press (look at object) | Нет звука | — | — | Silence при hover |
| Hold-E progress | Нет звука, только visual | — | — | Commit = pencil scratch |
| Hold-E complete | Один тихий "thud" | -18 dB rel ambient | 40 ms | Подтверждение без fanfare |

Принцип гиперреальности: звуки взаимодействия (бумага, карандаш, ручка) намеренно микшируются громче ambient. Это создаёт intimacy — как в ASMR-записях тихой работы. Игрок инстинктивно наклоняется к экрану. Это нужно.

### Layer 3 — Critical Scripted Sounds (однократные)

**Telephone:**
```
Tone:        425 Hz (советский/европейский стандарт, не американский 440 Hz)
Ring cycle:  1.5s tone → 3.5s silence → повтор
Count:       5–6 гудков до подъёма трубки
Volume:      -8 dB rel ambient (телефон слышен чётко через комнату)
Reverb:      комнатный reverb пространства где стоит телефон, wet 40%
```

**Silero TTS Announcement:**
```
Pre-EQ:     highpass 300 Hz + lowpass 4000 Hz (радиотрактовка)
Volume:     -4 dB rel ambient (доминирует, как должно быть у трансляции)
Reverb:     RadioBroadcast preset + дополнительный room reverb помещения, wet 25%
Обрыв:      на полуслове, abrupt cut без fade — просто стоп
Post-cut:   60 Hz hum на 2 секунды звучит особенно громко (masking effect пропал)
```

**Final Piano Note:**
```
Нота:        D3 (ре малой октавы) — не высокое, не низкое. Resonant, неоднозначное.
             Не минор, не мажор в изоляции — одиночная нота.
Attack:      0.0 ms (acoustic piano: мгновенный удар молоточка)
Decay:       2.0 s
Sustain:     -∞ (натуральный piano decay, без sustain pedal)
Release:     3.5 s natural fade
Volume:      начинает с -20 dB, crescendo до -6 dB за первые 0.8 s
Reverb:      Large Hall, wet 70% — нота звучит в огромном пространстве, которого нет в комнате
Fade out:    начинается на 4.0 s, полностью -∞ к 8.0 s
```

D3 потому что ниже "тревожного" регистра, но с достаточной body. Финальный звук не должен читаться как "победа" или "трагедия" — только вес.

**Sirena (концовка 2):**
```
Start freq:  440 Hz
End freq:    880 Hz
Ramp time:   4.0 s (linear pitch ramp)
Pulse:       on/off 0.5s/0.5s модуляция амплитуды поверх ramp
Volume:      -6 dB, crescendo до 0 dB (full) к секунде 3
Reverb:      Large Room, wet 50%
```

**Silence (концовка 1 — full mixdown):**
```
Fade sequence:
  0.0 s: начало fadeout всех Layer 1 sources
  2.0 s: Layer 1 на -40 dB (практически ноль)
  3.0 s: Layer 2 off
  4.0 s: полная тишина
  4.5 s: piano note D3 входит
```
Именно пауза между последним ambient-звуком и piano note должна быть 0.5–1.0 s абсолютной тишины. Это самый важный тайминг во всей игре.

### Layer 4 — Spatial / Positional

**Reverb zones по помещениям:**

| Помещение | Preset base | RT60 | Pre-delay | Low freq rolloff | Notes |
|---|---|---|---|---|---|
| Шлюз | SmallRoom | 0.35 s | 8 ms | -3 dB @ 200 Hz | Низкий потолок, мягкие поверхности |
| Коридор | MediumRoom + тоннельный EQ | 0.65 s | 18 ms | flat | Длинный резонанс, отражение от стен |
| Секция В | LargeRoom + metallic | 0.90 s | 25 ms | +2 dB @ 150 Hz | Трубы, бетон, металл — живой reverb |
| Control Room | MediumRoom + dead | 0.50 s | 12 ms | -6 dB @ 300 Hz | Много оборудования, поглощение |

UE5: Audio Volume actors с Reverb Effect asset. Transition time между зонами = 1.5 s crossfade. Не мгновенное переключение — ухо слышит смену reverb резко.

3D Spatial Sources: каждый fluorescent fixture, каждая machine в Секции В. Ambient Sound Actor с Attenuation Settings, Shape = Sphere, Falloff = Logarithmic (не Linear — логарифмический ближе к физике). Outer Radius = 400 cm, Inner Radius = 80 cm.

---

## 3. Visual / Post-process Timeline

### Базовые Lighting Parameters

```
Directional Light:       ОТСУТСТВУЕТ (нет sun/sky direction)
Sky Light:               Intensity 0.0 (темно снаружи, ночь)
Lumen Global Illumination: ON
  Final Gather Quality:  High
  Lumen Scene Detail:    Medium (баланс quality/perf)
Indirect Lighting Intensity: 0.6 (приглушённый indirect, не физически корректный — намеренно flat)
Emissive:                лампы как единственный источник

Exponential Height Fog:
  Fog Density:           0.015
  Volumetric Fog:        ON
  Scattering Distribution: 0.5
  Albedo:                (0.9, 0.92, 0.95) — чуть холодный туман
  Emissive:              (0.0, 0.0, 0.0) — не светящийся
  Extinction Scale:      1.2
```

Volumetric fog делает лучи от флуоресцентных ламп видимыми — это важная деталь. АЭС в 3 часа ночи, пыль в воздухе.

### Fluorescent Flicker Blueprint

```
Flicker frequency base:    0.143 Hz (один цикл раз в ~7 секунд)
Dark frame duration:       0.05 s (3 кадра при 60fps)
Amplitude range:           Intensity от 0.85 до 1.0 (не полное выключение)
Randomization:             ±30% от base frequency, независимо для каждой лампы
Escalation trigger:        через игровой event в минуту 20: frequency → 0.4 Hz
Pre-climax (мин. 30-35):   frequency → 1.2 Hz, иногда полное выключение на 0.15 s
```

Важно: каждая лампа — independent random seed. Не sync'd мигание (это выглядит как анимация). Несинхронность создаёт органическое ощущение деградации электрики.

### Post-process Ramp — Timeline

Все рампы управляются через Material Parameter Collection или Timeline в Level Blueprint с плавными кривыми.

**Chromatic Aberration:**
```
0 мин:     0.0
5 мин:     0.5%  (Intensity = 0.5)
15 мин:    1.5%  (Intensity = 1.5)
25 мин:    3.0%  (Intensity = 3.0)
30 мин:    4.5%  (Intensity = 4.5)
35 мин:    5.0%  (Intensity = 5.0)
Curve:     EaseInOut на каждом сегменте (не linear)
```

5% — это граница между "едва заметно в движении" и "явно видно на белых краях". 5% на светлых советских стенах будет очень читаться.

**Film Grain:**
```
0 мин:     Intensity = 0.15, Size = 1.0
10 мин:    Intensity = 0.25
20 мин:    Intensity = 0.40
30 мин:    Intensity = 0.60
35 мин:    Intensity = 0.80, Size = 1.4 (зерно крупнее к финалу)
```

**Saturation / Color Grading:**
```
0 мин:     Saturation = 1.0, Temp = 4200K (советский зелёный палитра)
10 мин:    Saturation = 0.90
20 мин:    Saturation = 0.78
30 мин:    Saturation = 0.65
33 мин:    Saturation = 0.60, Temp начинает shift к 3800K (теплее — аномальный)
34 мин:    Temp = 3500K (warm orange tint — финальный выбор)
35 мин:    Saturation = 0.55 (концовка 1) или возврат к 0.60 (концовка 2)
```

Температурный shift на минуте 33-34 — единственный момент когда палитра становится теплой. Это должно читаться как нечто принципиально другое, нарушение установленного визуального языка. Советский зелёный + холодный белый — это "норма". Теплый оранжевый — это "что-то изменилось".

**Vignette:**
```
Intensity:   0.35 (статично на протяжении всей игры)
Roundness:   0.5
```

Мягкая постоянная виньетка усиливает туннельное зрение без нарастания. Нарастающая виньетка — дешёвый прием, игрок сразу считывает "что-то случается". Статичная работает как тихий давящий фрейм.

**DOF:**
```
Method:     Circle DOF (не Temporal — для FPP)
Focal Length: 35 mm (эквивалент, соответствует FOV 75 примерно)
Aperture (f-stop): f/2.8 — shallow depth
Focus distance: 180 cm (фокус на объектах перед игроком на расстоянии руки)
Near blur:  disabled (руки не размываются)
Far blur:   начинается от 350 cm, soft
```

При f/2.8 задний план (дальний конец коридора) будет в мягком blur. Это не кинематографично — это добавляет ощущение неопределённости что там. Близкие объекты взаимодействия в фокусе.

**Bloom:**
```
Method:         Convolution Bloom (UE5 native)
Intensity:      0.8
Size:           1.0
Threshold:      1.2 (только яркие источники blooming)
```

Bloom на флуоресцентных лампах создаёт их характерный "ореол". На экранах мониторов в Control Room — threshold позволит им светиться. Не агрессивный bloom.

**Heat Distortion (Секция В):**

Реализуется через Niagara с Refraction material, не через post-process volume:
```
Location:        у горячих труб, первые 60 cm над трубой
Intensity:       0.008 (Refraction Index offset) — еле видимое дрожание
Coverage:        полоса 15 cm шириной над каждой горячей трубой
Update rate:     60 Hz, noise-based distortion
```

---

## 4. Interaction Feedback Design

### E to Interact — Instant Action

Мгновенный response без задержки. Никакого hold для простых действий (переключатели, подбор документа, посмотреть на что-то). Press → world state changes.

Feedback stack на instant interact:
1. Механический щелчок / звук объекта (-4 dB)
2. Micro camera nudge: +0.3° pitch вниз на 80 мс, EaseOut возврат за 200 мс (как будто рука потянулась)
3. Никакого UI toast

### Hold-E Journal Entry — 1.5 секунды

**Visual feedback: минималистичный подход без UI.**

Вместо progress bar — рука (FPP arm) начинает медленно тянуться к журналу. Это диегетично. Если нет рук (pure camera FPP), то: объект в фокусе начинает subtle highlight intensify (outline brightness + 30% за 1.5 секунды).

Pencil scratch loop запускается немедленно при начале hold. Прерывается моментально при отпускании.

Commit (1.5 s reached):
- Pencil scratch fade out 100 мс
- Один тихий "thud" (-18 dB)
- Outline на объекте гаснет

Abort (отпустил раньше):
- Scratch stop немедленно
- Никакого звука
- Мир не изменился

### Object Outline / Hover Feedback

```
Style:             Geometric edge highlight (depth-based, не силуэт)
Color:             (0.92, 0.93, 0.88) — очень бледный теплый белый, едва отличим от стены
Width:             1.5 px (не "игровой" жирный outline — почти незаметный)
Fade-in time:      0.4 s EaseOut при наведении
Fade-out time:     0.25 s при уводе взгляда
Distance cutoff:   200 cm — дальше объект не подсвечивается (нужно подойти)
```

Firewatch использовал этот принцип: outline существует, но не кричит. Игрок понимает что объект интерактивен без иконки или text prompt.

**Нет HUD вообще.** Никакого crosshair. Никакого "Press E to interact". Никакого inventory indicator. Единственный диегетический UI — журнал при Tab. Журнал — физический объект в мире, не UI overlay.

### Failure State

Никакого game over, никакого "WRONG" popup. Ошибка в записи читается только при следующем открытии журнала — галочка там, но данные неверные. Это остаётся с игроком. Ненарративный guilt — самый тихий вид horror.

---

## 5. Tension Curve — Sound + Visual по Фазам

| Временной отрезок | Помещение | Ambient | Chromatic Ab. | Saturation | Flicker Freq | Ключевое событие |
|---|---|---|---|---|---|---|
| 0–3 мин | Шлюз | Full L1 + L2 | 0% | 100% | 0.143 Hz | Baseline. Игрок осваивается. |
| 3–10 мин | Коридор | Full | 0.5–1.0% | 95–90% | 0.143 Hz | Первая запись в журнал. Footsteps на линолеуме. |
| 10–20 мин | Коридор → Секция В | Full | 1.0–2.5% | 90–78% | 0.143 Hz → 0.2 Hz | Reverb в коридоре subtly deeper. Один прибор звучит чуть иначе (pitch -3%). |
| 20–30 мин | Секция В | L1 прерывается | 2.5–4.5% | 78–65% | 0.2–0.4 Hz | **60 Hz hum пропадает на 3.2 секунды.** Это главное событие. Потом возвращается. |
| 30–33 мин | Control Room | Ослабевает | 4.5–5.0% | 65–60% | 0.4–1.2 Hz | Лампы мигают чаще. Footsteps echo longer. |
| 33–35 мин | Control Room | Fade | 5.0% | 60% → temp shift | Нерегулярный | Финальный выбор. Temp смещается к теплому. |
| Концовка 1 | — | Full fadeout | — | 55% | Off | Тишина 0.5 s → D3 |
| Концовка 2 | — | Sirena | — | — | — | Sirena ramp 4 s |

**Момент на минуте 20 — 60 Hz hum пропадает:**

Это самое пугающее событие в игре, и в нём нет ничего видимого. 3.2 секунды абсолютной тишины после того как низкочастотный фон, который игрок перестал замечать (masking effect), внезапно уходит. Мозг интерпретирует это как "что-то сломалось". Возврат hum'а через 3.2 s — облегчение, но уже не такое же как раньше. Этот прием называется "negative space horror" — ужас через отсутствие. Vlambeer использовал его в звуковых событиях, Jan Willem Nijman говорил о silence как об инструменте в "The Art of Screenshake" (2012, GDC Vault).

---

## 6. TTS Pipeline Integration Spec

### Silero TTS v5

**Модель:** `v5_cis.pt` (Silero v5, CIS Russian pack) — мужской institutional голос `aidar` или `baya` (aidar глубже, baya чуть выше). Рекомендую `aidar` для диктора АЭС.

**CLI / Python генерация:**

```python
import torch

model, _ = torch.hub.load(
    repo_or_dir='snakers4/silero-models',
    model='silero_tts',
    language='ru',
    speaker='v4_ru'
)

text = "Внимание. Плановая проверка систем безопасности завершена."
audio = model.apply_tts(
    text=text,
    speaker='aidar',
    sample_rate=48000,
    put_accent=True,
    put_yo=True
)

import soundfile as sf
sf.write('s_announcement_01.wav', audio, 48000)
```

Silero v5 работает через тот же hub. Проверь наличие `v5_cis.pt` в кэше torch.hub — возможно нужен explicit model_url для v5.

**Post-processing каждого .wav перед импортом в UE:**

```
1. Normalize to -3 dBFS
2. Highpass 300 Hz (убрать низы, имитация speaker)
3. Lowpass 3500 Hz (убрать воздух и присутствие)
4. Slight saturation / harmonic distortion +8% (speaker artifact)
5. Export: 48000 Hz, 24-bit PCM WAV
```

Обработка в Audacity (free) или ffmpeg + sox. Результат — голос как из советского громкоговорителя, не как запись студии.

**File naming:**

```
Content/Audio/Voice/
  s_announcement_01.wav    — первое объявление (плановая проверка)
  s_announcement_02.wav    — второе (если есть)
  s_announcement_cut.wav   — оборванная версия (обрыв на полуслове)
```

**UE5 Asset setup:**

1. Импорт как `Sound Wave`, Compression: `PCM` (не ADPCM — качество важнее размера для ключевых аудио).
2. Создать `Sound Cue` для каждого файла.
3. В Sound Cue: добавить `Reverb` node перед Output, preset = `RadioBroadcast` (или custom preset).
4. Spatial: для трансляции через speaker — Attenuation Settings с Inner Radius = 50 cm, Falloff = 150 cm, Shape = Sphere. Звук не должен слышаться через всё здание — только в помещении с говорящим speaker'ом.

**RadioBroadcast custom Reverb в UE5:**

```
Room:          -1800 mB
Room HF:       -600 mB
Decay Time:    0.6 s
Decay HF Ratio: 0.5
Reflections:   -1200 mB
Reflections Delay: 0.005 s
Reverb:        -800 mB
Reverb Delay:  0.012 s
HF Reference:  3000 Hz
Diffusion:     40%
Density:       60%
```

---

## 7. Asset Shopping List

### Niagara Systems (из Niagara Examples Pack UE5.7, Marketplace Free)

| System | Использование | Настройка |
|---|---|---|
| `NS_Dust` / Ambient Dust | Пыль в воздухе под лампами | Scale down 60%, emission rate 2-5 particles/s, gravity off |
| `NS_Steam` или Ribbon | Heat distortion у труб | Заменить рендер на Refraction material, не smoke sprite |
| `NS_Sparks` | Необязательно — возможно мигающий контакт в Секции В | Редкий event-triggered, не loop |
| Volumetric Fog Beam | Уже через Exponential Height Fog | Native, не Niagara |

Конкретные имена Niagara assets варьируются по версии пака — искать по тегам Ambient, Dust, Environment.

### Sonniss GDC Audio Pack (Free, CC0 ежегодный релиз)

Категории для поиска в архиве (sonniss.com/gameaudiogdc):

| Category | Query | Для чего |
|---|---|---|
| `industrial_mechanical` | "hum", "electrical", "generator" | 60 Hz hum, HVAC |
| `ambience_interior` | "corridor", "room tone", "office" | Room tone layers |
| `foley_footsteps` | "linoleum", "hard floor", "concrete" | Footsteps |
| `mechanical_switch` | "click", "relay", "button" | Handle click, interact confirm |
| `door` | "metal", "heavy", "creak" | Soviet door |

### Freesound (CC0 / CC-BY, freesound.org)

| Search query | License target | Для чего |
|---|---|---|
| "fluorescent hum" | CC0 | Fluorescent buzz layer |
| "electrical hum 60hz" | CC0 | Grid hum base |
| "linoleum footsteps" | CC0 | Footsteps base samples |
| "paper rustle" | CC0 | Journal page turn |
| "pencil writing paper" | CC0 | Pencil scratch |
| "dial tone 425hz" | CC0 | Soviet telephone tone |
| "ventilation hum" | CC0 | HVAC rumble |

Фильтровать: Licenses → "Creative Commons 0". Собирать минимум 4-6 вариантов на каждый sound type для рандомизации.

### BBC Sound Effects (bbcsfx.ac.uk — free для personal/non-commercial)

| Search | Для чего |
|---|---|
| "telephone ringing Soviet" / "telephone ringing Europe" | Telephone ring variants |
| "office ambience" | Control room base tone |
| "factory ambience" | Секция В atmosphere |

Внимание: BBC SFX лицензия = Royalty Free для personal use, но нужно проверить commercial terms если игра будет продаваться. При сомнении — заменить на Freesound CC0 аналоги.

---

## 8. Open Questions для Других Агентов

### Для game-designer

1. **Аномалия-триггер на минуту 20:** 60 Hz hum пропадает на 3.2 секунды. Это требует конкретного нарративного объяснения или остаётся необъяснённым? Необъяснённость сильнее, но нужна твоя позиция.

2. **Количество объявлений TTS:** Сколько announcement'ов всего и в каких помещениях? Feel-план предполагает минимум 2 (один полный, один оборванный). Если больше — нужна схема.

3. **Концовки:** Есть ли что-то между концовкой 1 (тишина + пианино) и концовкой 2 (сирена)? Или строго бинарно? От этого зависит sound design финального акта.

4. **Failure state gravity:** Игрок сделал ошибочную запись. Это вообще как-то отражается в мире (anomaly эскалирует быстрее?) или только нарративно (он несёт это с собой)? Feel-решение зависит от механической связи.

### Для level-designer

1. **Размеры коридора:** Reverb RT60 = 0.65 s предполагает длинный узкий коридор 15-20 м. Если коридор короче — RT60 нужно уменьшить до 0.45-0.50 s.

2. **Расположение флуоресцентных ламп:** Каждая лампа = отдельный 3D Ambient Sound Actor. Нужна схема fixture placement чтобы audio artist не расставлял наугад.

3. **Горячие трубы в Секции В:** Heat distortion у труб — нужны конкретные позиции труб в layout'е для Niagara placement.

4. **Переходные зоны между помещениями:** Audio Volume (reverb zone) переключается с crossfade 1.5 s. Где конкретно ставить boundaries? У дверей, или с overlap зоной между помещениями?

---

*Документ — feel-spec v1.0. Sub-параметры (dB, timing, Hz) фиксируются как hypothesis — финальные значения после первого playthrough сессии.*
