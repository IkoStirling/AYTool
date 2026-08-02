# AYTool

Offline CLIs for the Aliyat content pipeline. **Business logic stays in libraries**
(`AYResource`, `AYStorage`); this module only hosts executables.

| EXE | Role |
|-----|------|
| `cook_tool` | **P4 primary** — cooked `.ay*` tree → `content.pak` + `resources.db` |
| `package_tool` | Low-level pak writer (`IPackageWriter`) |
| `index_tool` | Scan helper (`IResourceMetaIndex`; not the ship DB) |

## cook_tool

```bat
cook_tool --assets <cooked_assets_root> --out <ship_dir> [-c zstd]
```

Runtime mount:

```cpp
ayt::resource::ResourceManager::instance().openDatabase("ship/resources.db");
// then load<T>("meshes/hero.aymesh") prefers pak via DB
```

Core API: `AYResource/include/AYCookShip.h` → `cookShipPackage()`.
