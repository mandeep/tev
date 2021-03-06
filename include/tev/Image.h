// This file was developed by Thomas Müller <thomas94@gmx.net>.
// It is published under the BSD 3-Clause License within the LICENSE file.

#pragma once

#include <tev/Channel.h>
#include <tev/GlTexture.h>
#include <tev/SharedQueue.h>
#include <tev/ThreadPool.h>

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

TEV_NAMESPACE_BEGIN

class ImageLoader;

struct ImageData {
    std::vector<Channel> channels;
    std::vector<std::string> layers;
};

class Image {
public:
    Image(const filesystem::path& path, const std::string& channelSelector);

    const filesystem::path& path() const {
        return mPath;
    }

    const std::string& channelSelector() const {
        return mChannelSelector;
    }

    const std::string& name() const {
        return mName;
    }

    std::string shortName() const;

    bool hasChannel(const std::string& channelName) const {
        return channel(channelName) != nullptr;
    }

    const Channel* channel(const std::string& channelName) const {
        auto it = std::find_if(std::begin(mData.channels), std::end(mData.channels), [&channelName](const Channel& c) { return c.name() == channelName; });
        if (it != std::end(mData.channels)) {
            return &(*it);
        } else {
            return nullptr;
        }
    }

    const GlTexture* texture(const std::vector<std::string>& channelNames);

    std::vector<std::string> channelsInLayer(std::string layerName) const;

    Eigen::Vector2i size() const {
        return mData.channels.front().size();
    }

    Eigen::DenseIndex count() const {
        return mData.channels.front().count();
    }

    const std::vector<std::string>& layers() const {
        return mData.layers;
    }

    int id() const {
        return mId;
    }

    std::string toString() const;

private:
    static std::atomic<int> sId;

    void ensureValid();

    filesystem::path mPath;
    std::string mChannelSelector;

    std::string mName;

    std::map<std::string, GlTexture> mTextures;

    ImageData mData;

    const int mId;
};

std::shared_ptr<Image> tryLoadImage(filesystem::path path, std::string channelSelector);

struct ImageAddition {
    bool shallSelect;
    std::shared_ptr<Image> image;
};

class BackgroundImagesLoader {
public:
    void enqueue(const filesystem::path& path, const std::string& channelSelector, bool shallSelect);
    ImageAddition tryPop() { return mLoadedImages.tryPop(); }

private:
    // A single worker is enough, since parallelization will happen _within_ each image load.
    ThreadPool mWorkers{1};
    SharedQueue<ImageAddition> mLoadedImages;
};

TEV_NAMESPACE_END
