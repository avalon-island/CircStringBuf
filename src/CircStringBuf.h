/*
 * CircStringBuf -- library implements fast circular buffer for C-style
 *                  (i.e, '\0'-terminated) strings.
 *
 * Arduino Class
 *
 * Copyright © 2019 sven-hm@github
 * Copyright © 2025 Andrei Kolchugin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

/* Rudimentary form of configuration */
#if defined(HAVE_CONFIG_H)
#	include "config.h"
#else /* defined(HAVE_CONFIG_H) */
#	if __has_include("config.h")
#		include "config.h"
#	endif /* __has_include("config.h") */
#endif /* defined(HAVE_CONFIG_H) */

#include <ThreadCompat.h>
#include <circstringbuf/circstringbuf.h>

class CircularStringBuffer {

public:

	CircularStringBuffer() = delete;
	CircularStringBuffer(size_t size);
	~CircularStringBuffer();

	inline int reset(void) {
	thread::ScopedMutexLock lock(mtxCSBuffer);

		return circstringbuf_reset(&bufferCtl);
	}

	int fillLevel(void) { return circstringbuf_filllevel(&bufferCtl); }

	int checkFit(size_t size) { return circstringbuf_checkfit(&bufferCtl, size); }
	int push(const char *string) {
	thread::ScopedMutexLock lock(mtxCSBuffer);

		return circstringbuf_push(&bufferCtl, string);
	}

	int strlen(size_t &size) {
	thread::ScopedMutexLock lock(mtxCSBuffer);

		return circstringbuf_strlen(&bufferCtl, &size);
	}
	int pop(char *string) {
	thread::ScopedMutexLock lock(mtxCSBuffer);

		return circstringbuf_pop(&bufferCtl, string);
	}
	int drop(void) {
	thread::ScopedMutexLock lock(mtxCSBuffer);

		return circstringbuf_drop(&bufferCtl);
	}

protected:

	thread::Mutex mtxCSBuffer;

private:

	circstringbuf_t bufferCtl;
	char *buffer;
};

template<size_t BufferSize>
class StaticCircularStringBuffer {

public:

	StaticCircularStringBuffer() : bufferCtl({buffer, BufferSize, 0, 0, true}) {}
	~StaticCircularStringBuffer() {}

	inline int reset(void) {
	thread::ScopedMutexLock lock(mtxCSBuffer);

		return circstringbuf_reset(&bufferCtl);
	}

	int fillLevel(void) { return circstringbuf_filllevel(&bufferCtl); }

	int checkFit(size_t size) { return circstringbuf_checkfit(&bufferCtl, size); }
	int push(const char *string) {
	thread::ScopedMutexLock lock(mtxCSBuffer);

		return circstringbuf_push(&bufferCtl, string);
	}

	int strlen(size_t &size) {
	thread::ScopedMutexLock lock(mtxCSBuffer);

		return circstringbuf_strlen(&bufferCtl, &size);
	}
	int pop(char *string) {
	thread::ScopedMutexLock lock(mtxCSBuffer);

		return circstringbuf_pop(&bufferCtl, string);
	}
	int drop(void) {
	thread::ScopedMutexLock lock(mtxCSBuffer);

		return circstringbuf_drop(&bufferCtl);
	}

protected:

	thread::Mutex mtxCSBuffer;

private:

	circstringbuf_t bufferCtl;
	char *buffer[BufferSize];
};

