/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#pragma once

#include <AzCore/Asset/AssetCommon.h>

#include <Atom/RPI.Reflect/Shader/ShaderCommonTypes.h>
#include <Atom/RPI.Reflect/Shader/ShaderVariantKey.h>

namespace AZ
{
    namespace RPI
    {
        class ShaderAsset2;
        class ShaderVariantTreeAsset;
        class ShaderVariantAsset2;

        //! This is the AZ::Interface<> declaration for the singleton responsible
        //! for finding the best ShaderVariantAsset a shader can use.
        //! This interface is public only to the ShaderAsset class.
        //! The expectation is that when in need of shader variants the developer
        //! should use AZ::RPI::Shader::GetVariant().
        class IShaderVariantFinder2
        {
        public:
            AZ_TYPE_INFO(IShaderVariantFinder2, "{4E041C2C-F158-412E-8961-76987EC75692}");

            static constexpr const char* LogName = "IShaderVariantFinder2";

            virtual ~IShaderVariantFinder2() = default;

            //! This function should be your one stop shop.
            //! It simply queues the request to load a shader variant asset.
            //! This function will automatically queue the ShaderVariantTreeAsset for loading if not available.
            //! Afther the ShaderVariantTreeAsset is loaded and ready, it is used to find the best matching ShaderVariantStableId
            //! from the given ShaderVariantId. If a valid ShaderVariantStableId is found, it will be queued for loading.
            //! Eventually the caller will be notified via ShaderVariantFinderNotificationBus::OnShaderVariantAssetReady()
            //! The notification will occur on the Main Thread.
            virtual bool QueueLoadShaderVariantAssetByVariantId(
                Data::Asset<ShaderAsset2> shaderAsset, const ShaderVariantId& shaderVariantId, SupervariantIndex supervariantIndex) = 0;

            //! This function does the first half of the work. It simply queues the loading of the ShaderVariantTreeAsset.
            //! Given the AssetId of a ShaderAsset it will try to find and load its corresponding ShaderVariantTreeAsset from
            //! the asset cache. If found, the asset will be loaded asynchronously and the caller will be notified via
            //! ShaderVariantFinderNotificationBus on main thread when the ShaderVariantTreeAsset is fully loaded.
            //! It is possible the requested ShaderVariantTreeAsset will never come into existence and in such
            //! case the caller will NEVER be notified.
            //! Returns true if the request was queued successfully.
            virtual bool QueueLoadShaderVariantTreeAsset(const Data::AssetId& shaderAssetId) = 0;

            //! This function does the second half of the work.
            //! Given the AssetId of a ShaderVariantTreeAsset and the stable id of a ShaderVariantAsset it will try to
            //! find its corresponding ShaderVariantAsset from the asset cache. If found, the asset will be loaded
            //! asynchronously and the caller will be notified via ShaderVariantFinderNotificationBus on main thread when the
            //! ShaderVariantAsset is fully loaded.
            //! Returns true if the request was queued successfully.
            virtual bool QueueLoadShaderVariantAsset(
                const Data::AssetId& shaderVariantTreeAssetId, ShaderVariantStableId variantStableId,
                SupervariantIndex supervariantIndex) = 0;

            //! This is a quick blocking call that will return a valid asset only if it's been fully loaded already,
            //! Otherwise it returns an invalid asset and the caller is supposed to call QueueLoadShaderVariantAssetByVariantId().
            virtual Data::Asset<ShaderVariantAsset2> GetShaderVariantAssetByVariantId(
                Data::Asset<ShaderAsset2> shaderAsset, const ShaderVariantId& shaderVariantId, SupervariantIndex supervariantIndex) = 0;

            virtual Data::Asset<ShaderVariantAsset2> GetShaderVariantAssetByStableId(
                Data::Asset<ShaderAsset2> shaderAsset, ShaderVariantStableId shaderVariantStableId, SupervariantIndex supervariantIndex) = 0;

            //! This is a quick blocking call that will return a valid asset only if it's been fully loaded already,
            //! Otherwise it returns an invalid asset and the caller is supposed to call QueueLoadShaderVariantTreeAsset().
            virtual Data::Asset<ShaderVariantTreeAsset> GetShaderVariantTreeAsset(const Data::AssetId& shaderAssetId) = 0;

            //! This is a quick blocking call that will return a valid asset only if i's been fully loaded already,
            //! Otherwise it returns an invalid asset and the caller is supposed to call QueueLoadShaderVariantAsset().
            virtual Data::Asset<ShaderVariantAsset2> GetShaderVariantAsset(
                const Data::AssetId& shaderVariantTreeAssetId, ShaderVariantStableId variantStableId,
                SupervariantIndex supervariantIndex) = 0;

            //! Clears the cache of loaded ShaderVariantTreeAsset and ShaderVariantAsset objects.
            //! This is intended for testing.
            virtual void Reset() = 0;
        };

        //! IShaderVariantFinder2 will call on this notification bus on the main thread.
        //! Only the following classes are supposed to register to this notification bus:
        //! AZ::RPI::ShaderAsset & AZ::RPI::Shader
        class ShaderVariantFinderNotification2
            : public EBusTraits
        {
        public:
            //////////////////////////////////////////////////////////////////////////
            // EBusTraits overrides
            static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
            using MutexType = AZStd::recursive_mutex;
            typedef Data::AssetId BusIdType; // The AssetId of the shader asset.
            //////////////////////////////////////////////////////////////////////////

            virtual void OnShaderVariantTreeAssetReady(Data::Asset<ShaderVariantTreeAsset> shaderVariantTreeAsset, bool isError) = 0;
            virtual void OnShaderVariantAssetReady(Data::Asset<ShaderVariantAsset2> shaderVariantAsset, bool isError) = 0;
        };
        using ShaderVariantFinderNotificationBus2 = AZ::EBus<ShaderVariantFinderNotification2>;

    } // namespace RPI
}// namespace AZ