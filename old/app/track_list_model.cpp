
#include "track_list_model.hpp"
#include "bar_item.hpp"

#include <algorithm>

#include <QDebug>

namespace securepath::drum {

track_list_model::track_list_model(QQmlContext& qml)
{
	qml.setContextProperty("trackListModel", this);
}

void track_list_model::set_song(drum::song* s, std::uint32_t sec) {
	beginResetModel();
	song_ = s;
	section_id_ = sec;
	section_ = song_->find_section(sec);
	endResetModel();
	Q_EMIT lengthChanged();
	Q_EMIT sectionChanged();
}

void track_list_model::on_instruments_changed() {
	beginResetModel();
	endResetModel();
}

void track_list_model::on_song_changed(std::uint32_t sec) {
	beginResetModel();
	section_id_ = sec;
	section_ = song_->find_section(sec);
	endResetModel();
	Q_EMIT lengthChanged();
	Q_EMIT sectionChanged();
}

Qt::ItemFlags track_list_model::flags(QModelIndex const& index) const {
	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

int track_list_model::columnCount(QModelIndex const& parent) const {
	return section_ ? section_->length() : 0;
}

int track_list_model::rowCount(QModelIndex const& parent) const {
	return section_ ? section_->tracks().size() : 0;
}

QModelIndex track_list_model::index(int row, int column, QModelIndex const& parent) const {
	QModelIndex ret;
	if(row < rowCount() && column < columnCount()) {
		ret = createIndex(row, column, nullptr);
	}
	return ret;
}

QModelIndex track_list_model::parent(QModelIndex const& child) const {
	return QModelIndex();
}

QVariant track_list_model::data(QModelIndex const& index, int role) const {
	QVariant ret;
	if(index.isValid()) {
		if(role == bar) {
			auto const& bars = section_->tracks()[index.row()].bars();
			time_signature sig = song_->default_time_signature();
			if(index.column() < bars.size()) {
				ret = QVariant::fromValue(bar_data(sig.beats_in_bar(), bars[index.column()]));
			} else {
				ret = QVariant::fromValue(bar_data(sig.beats_in_bar()));
			}
		} else {
			qDebug() << "track_list_model::data unknown role";
		}
	}
	return ret;
}

QHash<int, QByteArray> track_list_model::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[bar] = "bar";
	roles[division] = "division";
	return roles;
}

bool track_list_model::setMark(int row, int column, QVariant const& value) {
	QModelIndex index = this->index(row, column);
	bool ret = index.row() < rowCount();
	if(ret) {
		auto& bars = section_->tracks()[index.row()].bars();
		bar_data d = value.value<bar_data>();
		bars[index.column()] = d.data;
		Q_EMIT dataChanged(index, index);
	}
	return ret;
}

int track_list_model::length() const {
	return columnCount();
}

void track_list_model::setLength(int length) {
	int diff = length - columnCount();
	if(diff != 0 && columnCount() + diff > 0) {
		if(diff > 0) {
			beginInsertColumns(QModelIndex(), columnCount(), columnCount()+diff-1);
		} else {
			beginRemoveColumns(QModelIndex(), columnCount()+diff, columnCount()-1);
		}
		if(section_) {
			section_->set_length(length);
		}
		if(diff > 0) {
			endInsertColumns();
		} else {
			endRemoveColumns();
		}
		Q_EMIT lengthChanged();
	}
}

int track_list_model::section_id() const {
	return section_id_;
}

}
