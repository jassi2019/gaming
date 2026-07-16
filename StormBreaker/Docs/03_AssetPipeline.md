# StormBreaker — Asset Pipeline & Performance Guidelines

## Asset Pipeline

### Workflow
```
Concept Art → 3D Modeling → UV/Texturing → Rigging → Animation → Import → Blueprint → Test
                (Blender)    (Substance)   (Blender)  (Blender)   (UE5)    (UE5)     (UE5)
```

### Import Settings

#### Skeletal Meshes (Characters)
- Format: FBX 2020
- Scale: 1 UE unit = 1 cm
- Skeleton: Humanoid IK compatible
- LODs: 4 levels (auto-generated + hand-tuned LOD0)
- Collision: Capsule for movement, per-bone for hits

#### Static Meshes (Environment)
- Format: FBX 2020
- LODs: 3-4 levels
- Collision: Simple (box/convex hull), never complex on mobile
- Lightmap UV: Channel 1, minimum 64 resolution
- Nanite: Enable for PC (buildings, terrain), disable for mobile

#### Textures
| Platform | Max Resolution | Format |
|----------|---------------|--------|
| Android Low | 512x512 | ASTC 6x6 |
| Android High | 1024x1024 | ASTC 4x4 |
| Windows | 2048x2048 (props), 4096x4096 (terrain) | BC7 |

Texture Sets:
- BaseColor (sRGB, no alpha unless masked)
- Normal (Linear, DirectX format)
- ORM (Occlusion + Roughness + Metallic packed, Linear)
- Emissive (only when needed)

#### Audio
- Format: OGG Vorbis (mobile), WAV for editor
- Sample rate: 44.1 kHz
- Channels: Mono for 3D sounds, Stereo for music/UI
- Streaming: Enable for files > 500 KB

### Texture Streaming Budget (Mobile)
```
Total GPU memory target: 512 MB

Characters:     ~80 MB  (player + nearby enemies)
Weapons:        ~40 MB  (equipped + ground loot nearby)
Environment:    ~200 MB (terrain + buildings in view)
UI:             ~60 MB  (HUD textures)
Effects:        ~40 MB  (particles, decals)
Reserve:        ~92 MB  (headroom)
```

## Performance Guidelines

### Target Specifications

| Metric | Android (Mid) | Android (High) | Windows |
|--------|--------------|----------------|---------|
| FPS | 30 stable | 60 stable | 60-120 |
| Draw Calls | < 500 | < 800 | < 2000 |
| Triangles/frame | < 300K | < 700K | < 3M |
| RAM Usage | < 2 GB | < 3 GB | < 6 GB |
| APK Size | < 2 GB | < 2 GB | N/A |
| Texture Pool | 256 MB | 512 MB | 2 GB |

### Mobile Optimization Checklist

1. **Rendering**
   - Forward rendering (not Deferred) on mobile
   - No Lumen on mobile — use baked lighting + dynamic shadows for player only
   - No Nanite on mobile — manual LODs
   - MSAA 2x or TAA mobile variant
   - Half-resolution translucency
   - Planar reflections only for water (no SSR on mobile)

2. **Materials**
   - Max 128 instruction count for mobile materials
   - Avoid dynamic branching in pixel shaders
   - Use Material Instances for parameter variation (not unique materials)
   - Shared material parent per category (weapons, characters, environment)

3. **Particles (Niagara)**
   - Scalability levels: Low (50% particles), Medium (75%), High (100%)
   - GPU sim on PC, CPU sim on mobile
   - Max 500 concurrent particles on mobile
   - Use sprite sheets over mesh particles on mobile

4. **Physics**
   - Chaos physics with simplified collision
   - Max 50 active physics bodies on mobile
   - No cloth simulation on Low quality
   - Ragdoll: 8 bones max on mobile

5. **Audio**
   - Max 32 concurrent voices on mobile
   - Distance-based culling for gunshots
   - Low-pass filter for distant sounds
   - Compressed audio streaming

6. **Memory**
   - Texture streaming with aggressive pooling
   - Soft object references for deferred loading
   - Level streaming for map chunks
   - Garbage collection tuning: `gc.MaxObjectsNotConsideredByGC=200000`

### Level Streaming Strategy
```
┌──────────────────────────────────────┐
│           PERSISTENT LEVEL           │
│  (Terrain, Zone logic, Game systems) │
│                                      │
│  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐   │
│  │Cell │ │Cell │ │Cell │ │Cell │   │
│  │ A1  │ │ A2  │ │ B1  │ │ B2  │   │
│  │     │ │     │ │     │ │     │   │
│  └─────┘ └─────┘ └─────┘ └─────┘   │
│                                      │
│  Grid: 8x8 = 64 streaming cells     │
│  Load radius: 3 cells ahead          │
│  Unload: 5 cells behind              │
└──────────────────────────────────────┘
```

### Profiling Tools
- **Unreal Insights** — frame capture and analysis
- **Stat Unit** — game/render/GPU thread timing
- **Stat SceneRendering** — draw call count
- **Android GPU Inspector** — mobile GPU profiling
- **Memory Report** — `memreport -full` in console
- **Network Profiler** — bandwidth per actor class

### Build Configurations
| Config | Use |
|--------|-----|
| Development | Day-to-day coding with debug symbols |
| DebugGame | Deep debugging with full assertions |
| Test | Automated testing builds |
| Shipping | Final release, all optimizations enabled |
