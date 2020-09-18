/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Raspberry Pi (Trading) Ltd.
 *
 * rpi_stream.h - Raspberry Pi device stream abstraction class.
 */
#ifndef __LIBCAMERA_PIPELINE_RPI_STREAM_H__
#define __LIBCAMERA_PIPELINE_RPI_STREAM_H__

#include <queue>
#include <string>
#include <vector>

#include <libcamera/stream.h>

#include "libcamera/internal/v4l2_videodevice.h"

namespace libcamera {

namespace RPi {

/*
 * Device stream abstraction for either an internal or external stream.
 * Used for both Unicam and the ISP.
 */
class RPiStream : public Stream
{
public:
	RPiStream()
	{
	}

	RPiStream(const char *name, MediaEntity *dev, bool importOnly = false)
		: external_(false), importOnly_(importOnly), name_(name),
		  dev_(std::make_unique<V4L2VideoDevice>(dev))
	{
	}

	V4L2VideoDevice *dev() const;
	std::string name() const;
	bool isImporter() const;
	void reset();

	void setExternal(bool external);
	bool isExternal() const;

	void setExportedBuffers(std::vector<std::unique_ptr<FrameBuffer>> *buffers);
	const std::vector<FrameBuffer *> &getBuffers() const;
	bool findFrameBuffer(FrameBuffer *buffer) const;

	int prepareBuffers(unsigned int count);
	int queueBuffer(FrameBuffer *buffer);
	void returnBuffer(FrameBuffer *buffer);

	int queueAllBuffers();
	void releaseBuffers();

private:
	void clearBuffers();
	int queueToDevice(FrameBuffer *buffer);

	/*
	 * Indicates that this stream is active externally, i.e. the buffers
	 * might be provided by (and returned to) the application.
	 */
	bool external_;

	/* Indicates that this stream only imports buffers, e.g. ISP input. */
	bool importOnly_;

	/* Stream name identifier. */
	std::string name_;

	/* The actual device stream. */
	std::unique_ptr<V4L2VideoDevice> dev_;

	/* All frame buffers associated with this device stream. */
	std::vector<FrameBuffer *> bufferList_;

	/*
	 * List of frame buffers that we can use if none have been provided by
	 * the application for external streams. This is populated by the
	 * buffers exported internally.
	 */
	std::queue<FrameBuffer *> availableBuffers_;

	/*
	 * List of frame buffers that are to be queued into the device from a Request.
	 * A nullptr indicates any internal buffer can be used (from availableBuffers_),
	 * whereas a valid pointer indicates an external buffer to be queued.
	 *
	 * Ordering buffers to be queued is important here as it must match the
	 * requests coming from the application.
	 */
	std::queue<FrameBuffer *> requestBuffers_;

	/*
	 * This is a list of buffers exported internally. Need to keep this around
	 * as the stream needs to maintain ownership of these buffers.
	 */
	std::vector<std::unique_ptr<FrameBuffer>> internalBuffers_;
};

/*
 * The following class is just a convenient (and typesafe) array of device
 * streams indexed with an enum class.
 */
template<typename E, std::size_t N>
class RPiDevice : public std::array<class RPiStream, N>
{
private:
	constexpr auto index(E e) const noexcept
	{
		return static_cast<std::underlying_type_t<E>>(e);
	}
public:
	RPiStream &operator[](E e)
	{
		return std::array<class RPiStream, N>::operator[](index(e));
	}
	const RPiStream &operator[](E e) const
	{
		return std::array<class RPiStream, N>::operator[](index(e));
	}
};

} /* namespace RPi */

} /* namespace libcamera */

#endif /* __LIBCAMERA_PIPELINE_RPI_STREAM_H__ */