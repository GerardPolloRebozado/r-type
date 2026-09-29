/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** Asset.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_ASSET_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_ASSET_HPP


enum AssetType {
    Sound,
    Image
};

template<typename T>
class Asset {
    AssetType _type;
    T _asset;

public:
    T &get() const {
        return &_type;
    }

    AssetType type() const {
        return _type;
    }

    Asset(const AssetType type, T asset) : _type(type), _asset(asset) {
    };
};
#endif //G_ING_500_BAR_5_1_RTYPE_1_ASSET_HPP
