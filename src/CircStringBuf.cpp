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

#include <CircStringBuf.h>

CircularStringBuffer::CircularStringBuffer(size_t size)
	: bufferCtl({NULL, 0, 0, 0, true}) {

	if (buffer = new char(size)) {

		bufferCtl.start = buffer;
		bufferCtl.end = size;

		memset(buffer, 0, size);
	}
}

CircularStringBuffer::~CircularStringBuffer() {

	if (buffer)
		free(buffer);
}

