# Scene Serialization

Scenes can be saved to `.scene` JSON files. The `SceneSerializer` iterates over all entities and uses your `IPluginEditor` implementations to serialize and deserialize component data.
