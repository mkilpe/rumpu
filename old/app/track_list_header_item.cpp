
#include "track_list_header_item.hpp"
#include "track_list_model.hpp"

#include <QPainter>

namespace securepath::drum {

void track_list_header_item::paint(QPainter* painter) {
	track_list_model* model = static_cast<track_list_model*>(model_);
	if(model) {
		painter->fillRect(0, 0, size().width(), size().height(), QColor("lightgrey"));
	}
}

}

