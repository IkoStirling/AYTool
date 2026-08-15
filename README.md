# AYTool

Offline CLIs for the Aliyat content pipeline. **Business logic stays in libraries**
(`AYResource`, `AYStorage`); this module only hosts executables.

| EXE | Role |
|-----|------|
| `import_tool` | **P5 primary** — FBX/glTF/texture → cooked `.ay*` cache tree |
| `cook_tool` | **P4 primary** — cooked `.ay*` tree → `content.pak` + `resources.db` |
| `package_tool` | Low-level pak writer (`IPackageWriter`) |
| `index_tool` | Scan helper (`IResourceMetaIndex`; not the ship DB) |

## import_tool

```bat
import_tool --in <source.fbx> --out <assets_dir> [--force]
import_tool --in a.fbx --in b.fbx --out ayeditor_cache/assets
```

Aligns with `EditorShellDemo` / `EditorPlayRuntime::resolvePersistentCacheRoot()`:
`--out` should be the `assets\` directory under `ayeditor_cache`.

Core API: `AYResource/include/AYResource/ImportJob.h` → `importAsset()` / `importAssetBatch()`.

## cook_tool

```bat
cook_tool --assets <cooked_assets_root> --out <ship_dir> [-c zstd]
```

Runtime mount:

```cpp
ayt::resource::ResourceManager::instance().openDatabase("ship/resources.db");
// then load<T>("meshes/hero.aymesh") prefers pak via DB
```

Core API: `AYResource/include/AYResource/CookShip.h` → `cookShipPackage()`.
