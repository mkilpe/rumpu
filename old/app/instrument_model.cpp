#include "instrument_model.hpp"

namespace securepath::drum {

instrument_model::instrument_model(QQmlContext& qml)
{
	qml.setContextProperty("instrumentModel", this);
}

void instrument_model::set_song(drum::song* s) {
	beginResetModel();
	song_ = s;
	endResetModel();
}

void instrument_model::on_song_changed() {
	beginResetModel();
	endResetModel();
}

int instrument_model::rowCount(QModelIndex const& parent) const {
	return song_ ? song_->instruments().size() : 0;
}

QVariant instrument_model::data(QModelIndex const& index, int role) const {
	QVariant ret;
	if(index.isValid() && index.row() < rowCount()) {
		auto const& instr = song_->instruments()[index.row()];
		if(role == name) {
			ret = QString::fromStdString(instr.name());
		} else if(role == mute) {
			ret = instr.volume().mute;
		} else if(role == volume) {
			ret = instr.volume().value;
		}
	}
	return ret;
}

QHash<int, QByteArray> instrument_model::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[name] = "name";
	roles[mute] = "mute";
	roles[volume] = "volume";
	return roles;
}

void instrument_model::add(QString sample_url) {
	if(song_) {
		QUrl url(sample_url);
		beginInsertRows(QModelIndex(), rowCount(), rowCount());
		try {
			song_->add_instrument(instrument(url.toLocalFile().toStdString()));
		} catch(std::exception const& ex) {
			//t
		}
		endInsertRows();
		Q_EMIT instruments_changed();
	}
}


Qt::ItemFlags instrument_model::flags(QModelIndex const& index) const {
	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool instrument_model::setData(QModelIndex const& index, QVariant const& value, int role) {
	bool ret = false;
	if(index.isValid() && index.row() < rowCount()) {
		auto& instr = song_->instruments()[index.row()];
		if(role == mute) {
			drum::volume v = instr.volume();
			v.mute = value.toBool();
			instr.set_volume(v);
			Q_EMIT dataChanged(index, index);
			ret = true;
		} else if(role == volume) {
			drum::volume v = instr.volume();
			v.value = value.toFloat();
			instr.set_volume(v);
			Q_EMIT dataChanged(index, index);
			ret = true;
		}
	}
	return ret;
}

}
