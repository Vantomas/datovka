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

#include <QColorDialog>
#include <QMessageBox>

#include "src/gui/dlg_tag.h"
#include "src/web/json.h"

DlgTag::DlgTag(const QString &userName, TagDb *tagDb,
    bool isWebDatovkaAccount, QWidget *parent)
    : QDialog(parent),
    m_userName(userName),
    m_tagDbPtr(tagDb),
    m_isWebDatovkaAccount(isWebDatovkaAccount),
    m_tagItem()
{
	setupUi(this);

	initDlg();
}

DlgTag::DlgTag(const QString &userName, TagDb *tagDb,
    bool isWebDatovkaAccount, const TagItem &tag, QWidget *parent)
    : QDialog(parent),
    m_userName(userName),
    m_tagDbPtr(tagDb),
    m_isWebDatovkaAccount(isWebDatovkaAccount),
    m_tagItem(tag)
{
	setupUi(this);

	initDlg();
}

void DlgTag::initDlg(void)
{
	this->currentColor->setEnabled(false);
	this->tagNamelineEdit->setText(m_tagItem.name);
	setPreviewButtonColor();

	connect(this->changeColorPushButton, SIGNAL(clicked()), this,
	    SLOT(chooseNewColor()));
	connect(this->buttonBox, SIGNAL(accepted()), this,
	    SLOT(saveTag()));
}

void DlgTag::chooseNewColor(void)
{
	QColor colour = QColorDialog::getColor(QColor("#" + m_tagItem.colour),
	    this, tr("Choose tag colour"));

	if (colour.isValid()) {
		QString colourName = colour.name().toLower().replace("#", "");
		if (TagItem::isValidColourStr(colourName)) {
			m_tagItem.colour = colourName;
		}
		setPreviewButtonColor();
	}
}

void DlgTag::saveTag(void)
{
	m_tagItem.name = this->tagNamelineEdit->text();

	if (m_tagItem.name.isEmpty()) {
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setWindowTitle(tr("Tag error"));
		msgBox.setText(tr("Tag name is empty."));
		msgBox.setInformativeText(tr("Tag wasn't created."));
		msgBox.exec();
		return;
	}

	Q_ASSERT(TagItem::isValidColourStr(m_tagItem.colour));

	JsonLayer jsLayer;
	QString errStr;
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Critical);

	if (m_tagItem.id >= 0) {

		if (m_isWebDatovkaAccount) {
			Q_ASSERT(!m_userName.isEmpty());
			if (!jsLayer.updateTag(m_userName, m_tagItem.id,
			   m_tagItem.name, m_tagItem.colour, errStr)) {
				msgBox.setWindowTitle(tr("Tag update error"));
				msgBox.setText(tr("Tag with name '%1'' wasn't "
				    "updated in the WebDatovka database.").arg(
				    m_tagItem.name));
				msgBox.setInformativeText(errStr);
				msgBox.exec();
				return;
			}
		}
		m_tagDbPtr->updateTag(m_tagItem.id, m_tagItem.name,
		    m_tagItem.colour);

	} else {
		if (m_isWebDatovkaAccount) {
			Q_ASSERT(!m_userName.isEmpty());
			int tagId = jsLayer.createTag(m_userName,
			    m_tagItem.name, m_tagItem.colour, errStr);
			if (tagId <= 0) {
				msgBox.setWindowTitle(tr("Tag insert error"));
				msgBox.setText(tr("Tag with name '%1'' wasn't'"
				    " created in WebDatovka database.").arg(
				    m_tagItem.name));
				msgBox.setInformativeText(errStr);
				msgBox.exec();
				return;
			}
			if (!m_tagDbPtr->insertUpdateWebDatovkaTag(tagId,
			    m_tagItem.name, m_tagItem.colour)) {
				msgBox.setWindowTitle(tr("Tag error"));
				msgBox.setText(tr("Tag with name '%1'' already "
				   "exists in database.").arg(m_tagItem.name));
				msgBox.setInformativeText(
				    tr("Tag wasn't created again."));
				msgBox.exec();
			}
		} else {
			if (!m_tagDbPtr->insertTag(m_tagItem.name,
			    m_tagItem.colour)) {
				msgBox.setWindowTitle(tr("Tag error"));
				msgBox.setText(tr("Tag with name '%1'' already "
				    "exists in database.").arg(m_tagItem.name));
				msgBox.setInformativeText(
				    tr("Tag wasn't created again."));
				msgBox.exec();
			}
		}
	}
}

void DlgTag::setPreviewButtonColor(void)
{
	QPalette pal = this->currentColor->palette();
	pal.setColor(QPalette::Button, QColor("#" + m_tagItem.colour));
	const QString style = "border-style: outset; background-color: ";
	this->currentColor->setStyleSheet(style + "#" + m_tagItem.colour);
}
