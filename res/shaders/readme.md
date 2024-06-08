# Rules for using GPU registers:

## Buffer registers:
- Every shader should have an `object_buffer` bound to `b0 (VS)`
- **DO NOT OVERWRITE** `b0 (PS)` buffer which is declared in `lighting_calculations.hlsl`
- `b3 (PS)` is occupied by a misc buffer

## Sampler registers:
- `s0` is for the main texture sampler
- `s1` is for the shadow map sampler
- `s2` is for the fog sampler
- If we ever need to use different samplers, just bind them to the next slots.

## Texture Registers:
[DX11 has circa 128 texture registers according to this source.](https://gamedev.stackexchange.com/questions/158632/hlsl-registers-and-slots)
- `t0` is always the object's texture
- `t1` is for the directional shadow map
- `t10`, `t11`, `t12` are for deferred shading textures
- `t14` is for ambient occlusion
- `t15` is for the SkyBox resource
- `t16` is for the Fog texture
- `t18` is for water normal map #1
- `t19` is for water normal map #1
- `t20` to `t39` are for spotlight shadow maps
- `t40` to `t59` are for point shadow maps
- `t20` to `t39` are for spotlight shadow maps
- `t40` to `t59` are for point shadow maps

If you want to bind any new textures specific to an object, I suggest using `t2-t9` registers.
