/*
 * Copyright (C) 2014-2016 CZ.NIC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations including
 * the two.
 */

#ifndef _DIMENSIONS_H_
#define _DIMENSIONS_H_

#include <QStyleOptionViewItem>

/*!
 * @brief Defines some basic dimensions.
 */
class Dimensions {

public:
	static
	qreal margin; /*!< Text margin as ratio of text height. */

	static
	qreal padding; /*!< Text padding as ratio of text height. */

	static
	qreal lineHeight; /*!< Height of line as ration of text height. */

	/*!
	 * @brief Return default line height to table views and widgets.
	 *
	 * @param[in] option Style options.
	 * @return Line height pixels.
	 */
	static
	int tableLineHeight(const QStyleOptionViewItem &option);

private:
	/*!
	 * @brief Constructor.
	 *
	 * @note Prohibit any class instance.
	 */
	Dimensions(void);
};

#endif /* _DIMENSIONS_H_ */
