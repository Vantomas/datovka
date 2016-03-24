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

#include <QString>

#include "src/delegates/tags_delegate.h"
#include "src/gui/dlg_tag.h"
#include "src/gui/dlg_tags.h"
#include "src/io/tag_db.h"
#include "src/models/tags_model.h"

#define WRONG_TAG_ID -1 /** TODO -- Remove. */

DlgTags::DlgTags(QWidget *parent)
    : QDialog(parent),
    m_msgIdList(),
    m_tagsDelegate(0),
    m_tagsModel(0)
{
	setupUi(this);

	initDlg();
}

DlgTags::DlgTags(const QList<qint64> &msgIdList, QWidget *parent)
    : QDialog(parent),
    m_msgIdList(msgIdList),
    m_tagsDelegate(0),
    m_tagsModel(0)
{
	setupUi(this);

	initDlg();
}

DlgTags::~DlgTags(void)
{
	if (0 != m_tagsDelegate) {
		delete m_tagsDelegate;
	}
	if (0 != m_tagsModel) {
		delete m_tagsModel;
	}
}


void DlgTags::addTag(void)
{
	QDialog *tagDlg = new DlgTag(this);
	tagDlg->exec();
	tagDlg->deleteLater();

	fillTagsToListView();
}


void DlgTags::updateTag(void)
{
	TagItem tagItem(globTagDbPtr->getTagData(
	    getTagIdFromIndex(tagListView->selectionModel()->currentIndex())));

	QDialog *tagDlg = new DlgTag(tagItem, this);
	tagDlg->exec();
	tagDlg->deleteLater();

	fillTagsToListView();
}

void DlgTags::deleteTag(void)
{
	QModelIndexList slctIdxs(tagListView->selectionModel()->selectedRows());

	foreach (const QModelIndex &idx, slctIdxs) {
		globTagDbPtr->deleteTag(getTagIdFromIndex(idx));
	}

	fillTagsToListView();
}

void DlgTags::assignSelectedTagsToMsgs(void)
{
	QModelIndexList slctIdxs(tagListView->selectionModel()->selectedRows());

	foreach (const qint64 &msgId, m_msgIdList) {
		foreach (const QModelIndex &idx, slctIdxs) {
			globTagDbPtr->assignTagToMsg(getTagIdFromIndex(idx),
			    msgId);
		}
	}
}

void DlgTags::removeSelectedTagsFromMsgs(void)
{
	QModelIndexList slctIdxs(tagListView->selectionModel()->selectedRows());

	foreach (const qint64 &msgId, m_msgIdList) {
		foreach (const QModelIndex &idx, slctIdxs) {
			globTagDbPtr->removeTagFromMsg(getTagIdFromIndex(idx),
			    msgId);
		}
	}
}

void DlgTags::handleSelectionChanged(QItemSelection current)
{
	Q_UNUSED(current);

	QModelIndexList slctIdxs(tagListView->selectionModel()->selectedRows());

	if (slctIdxs.isEmpty()) {
		this->pushButtonUpdate->setEnabled(false);
		this->pushButtonDelete->setEnabled(false);
	} else {
		if (slctIdxs.count() > 1) {
			this->pushButtonUpdate->setEnabled(false);
		} else {
			this->pushButtonUpdate->setEnabled(true);
		}
		this->pushButtonDelete->setEnabled(true);
	}
}

void DlgTags::fillTagsToListView(void)
{
	TagItemList tagList(globTagDbPtr->getAllTags());

	m_tagsModel->setTagList(tagList);
}

void DlgTags::initDlg(void)
{
	m_tagsDelegate = new TagsDelegate(this);
	m_tagsModel = new TagsModel(this);

	tagListView->setItemDelegate(m_tagsDelegate);
	tagListView->setModel(m_tagsModel);

	tagListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	tagListView->setSelectionBehavior(QAbstractItemView::SelectRows);

	this->pushButtonUpdate->setEnabled(false);
	this->pushButtonDelete->setEnabled(false);

	this->tagAssignGroup->setEnabled(false);
	this->tagAssignGroup->setVisible(false);

	connect(this->pushButtonAdd, SIGNAL(clicked()), this,
	    SLOT(addTag()));
	connect(this->pushButtonUpdate, SIGNAL(clicked()), this,
	    SLOT(updateTag()));
	connect(this->pushButtonDelete, SIGNAL(clicked()), this,
	    SLOT(deleteTag()));
	connect(tagListView->selectionModel(),
	    SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
	    SLOT(handleSelectionChanged(QItemSelection)));


	/* any messages was selected */
	if (!m_msgIdList.isEmpty()) {

		connect(this->pushButtonAssign, SIGNAL(clicked()), this,
		    SLOT(assignSelectedTagsToMsgs()));
		connect(this->pushButtonRemove, SIGNAL(clicked()), this,
		    SLOT(removeSelectedTagsFromMsgs()));

		this->tagAssignGroup->setVisible(true);
		this->tagAssignGroup->setEnabled(true);
	}

	fillTagsToListView();
}

int DlgTags::getTagIdFromIndex(const QModelIndex &idx)
{
	if (!idx.isValid()) {
		return WRONG_TAG_ID;
	}

	if (!idx.data().canConvert<TagItem>()) {
		return WRONG_TAG_ID;
	}

	TagItem tagItem(qvariant_cast<TagItem>(idx.data()));

	return tagItem.id;
}
