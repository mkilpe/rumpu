#include "section_view_model.hpp"

namespace securepath::drum {

section_view_model::section_view_model(QQmlContext& qml)
{
	qml.setContextProperty("sectionViewModel", this);
}

void section_view_model::set_song(drum::song* s) {
	beginResetModel();
	song_ = s;
	endResetModel();
}

void section_view_model::on_song_changed() {
	beginResetModel();
	endResetModel();
}

Qt::ItemFlags section_view_model::flags(QModelIndex const& index) const {
	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

int section_view_model::rowCount(QModelIndex const& parent) const {
	return song_ ? song_->section_order().size() : 0;
}

QVariant section_view_model::data(QModelIndex const& index, int role) const {
	QVariant ret;
	if(index.isValid() && index.row() < rowCount()) {
		if(role == Qt::DisplayRole) {
			ret = song_->section_order()[index.row()];
		}
	}
	return ret;
}

bool section_view_model::setData(QModelIndex const& index, QVariant const& value, int role) {
	bool ret = false;
	if(index.isValid() && index.row() < rowCount()) {
		if(role == Qt::DisplayRole) {
			std::uint32_t sec_id = value.toInt();
			song_->section_order()[index.row()] = sec_id;
			if(!song_->find_section(sec_id)) {
				song_->add_section(sec_id);
			}
			Q_EMIT dataChanged(index, index);
			ret = true;
		}
	}
	return ret;
}

bool section_view_model::insertRows(int pos, int count, QModelIndex const& parent) {
	beginInsertRows(QModelIndex(), pos, pos+count-1);

	auto& c = song_->section_order();
	c.insert(c.begin()+pos, count, 0);

	endInsertRows();
	return true;
}

bool section_view_model::removeRows(int pos, int count, QModelIndex const& parent) {
	beginRemoveRows(QModelIndex(), pos, pos+count-1);

	auto& c = song_->section_order();
	c.erase(c.begin()+pos, c.begin()+pos+count);

	endRemoveRows();
	return true;
}

void section_view_model::append() {
	beginInsertRows(QModelIndex(), rowCount(), rowCount());

	song_->section_order().push_back(song_->add_section());

	endInsertRows();
}

void section_view_model::mouseClicked(int index) {
	Q_EMIT section_changed(song_->section_order()[index]);
}

void section_view_model::move(int source, int dest) {
	qDebug() << "hops" << source << dest;
	if(source < rowCount() && dest < rowCount()) {
		qDebug() << source << dest;
		beginResetModel();
		auto& list = song_->section_order();
		auto it = list.begin()+source;
		auto section = *it;
		list.insert(list.begin()+dest, section);
		list.erase(it);
		endResetModel();
	}
}

}
