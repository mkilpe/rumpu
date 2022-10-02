
#include "bar_item.hpp"

#include <QPainter>

#include <algorithm>

namespace securepath::drum {


QVariant bar_data::clear(bool only_data) {
	if(!data.beats.empty() && only_data) {
		for(auto& v : data.beats) {
			v.action = beat::none;
			if(!v.division.empty()) {
				for(auto& dv : v.division) {
					dv.action = beat::none;
				}
			}
		}
	} else {
		data = bar{};
	}
	return QVariant::fromValue(*this);
}

QVariant bar_data::toggle(float npos, beat::action_type toggle_action) {
	if(data.beats.empty()) {
		data.beats.resize(default_division);
	}
	int div = data.beats.size();
	int index = npos*div;
	if(index < data.beats.size()) {
		if(!data.beats[index].division.empty()) {
			float block_width = 1.0/div;
			float newpos = (npos - block_width*index) / block_width;
			int ndiv = data.beats[index].division.size();
			int nindex = newpos*ndiv;
			auto& vec = data.beats[index].division;
			if(nindex < vec.size()) {
				if(vec[nindex].action != beat::none) {
					vec[nindex].action = beat::none;
				} else {
					vec[nindex].action = toggle_action;
				}
			}
		} else {
			auto& action = data.beats[index].action;
			if(action != beat::none) {
				action = beat::none;
			} else {
				action = toggle_action;
			}
		}
	}
	return QVariant::fromValue(*this);
}

QVariant bar_data::toggle_hit(float npos) {
	return toggle(npos, beat::hit);
}

QVariant bar_data::toggle_stop(float npos) {
	return toggle(npos, beat::stop);
}

QVariant bar_data::divide(int div) {
	data = bar{};
	data.beats.resize(div);
	return QVariant::fromValue(*this);
}

int bar_data::division() const {
	return data.beats.empty() ? default_division : data.beats.size();
}

int bar_data::subdivision(int beat) const {
	int div = 1;
	if(beat > 0 && beat <= data.beats.size()) {
		if(!data.beats[beat-1].division.empty()) {
			div = data.beats[beat-1].division.size();
		}
	}
	return div;
}

QVariant bar_data::subdivide(int beat, int div) {
	if(data.beats.empty()) {
		data.beats.resize(default_division);
	}
	if(beat > 0 && beat <= data.beats.size()) {
		data.beats[beat-1].division.resize(div);
	}
	return QVariant::fromValue(*this);
}

namespace {
struct drawer {
	drawer(QPainter* p, bar_data const& data)
	: painter(*p)
	, data(data)
	, brush(QColor(0,0,0))
	, red_brush(QColor(255,0,0))
	{
		mark_pen.setColor(QColor(0, 150, 0, 100));
		mark_pen.setWidth(1);
		black_pen.setColor(QColor(0, 0, 0, 100));
		black_pen.setWidth(1);
		red_pen.setColor(QColor(150, 0, 0, 100));
		red_pen.setWidth(1);
		blue_pen.setColor(QColor(0, 0, 150, 100));
		blue_pen.setWidth(1);

		painter.setBrush(brush);
		painter.setRenderHint(QPainter::Antialiasing);

		hitsize = width / 12;
		hitsize = std::clamp(hitsize, 5, height/3);
	}

	void draw_mark(int x, bool accent, beat::action_type action) {
		if(x < width) {
			if(accent) {
				painter.drawLine(x, accent_mark_y_start, x, accent_mark_y_start+accent_mark_height);
			} else {
				painter.drawLine(x, accent_mark_y_start, x, accent_mark_y_start+mark_height);
			}
			if(action == beat::hit) {
				painter.drawEllipse(x-hitsize/2, height/2-hitsize/2, hitsize, hitsize);
			} else if(action == beat::stop) {
				painter.setBrush(red_brush);
				painter.drawEllipse(x-hitsize/2, height/2-hitsize/2, hitsize, hitsize);
				painter.setBrush(brush);
			}
		}
	}

	void draw() {
		if(!data.data.beats.empty()) {
			draw_marks_with_hits();
		} else if(data.default_division) {
			draw_default_marks();
		}
		painter.setPen(black_pen);
		painter.drawLine(0, 0, width, 0);
	}

	void draw_split(double x, int w, bool accent, std::vector<beat> const& div) {
		double inc = double(w) / div.size();
		painter.setPen(blue_pen);
		for(auto const& beat : div) {
			draw_mark(x, accent, beat.action);
			if(accent) {
				accent = false;
			}
			x += inc;
		}
	}

	void set_mark_pen() {
		if(data.data.beats.size() == data.default_division) {
			painter.setPen(mark_pen);
		} else {
			painter.setPen(red_pen);
		}
	}

	void draw_marks_with_hits() {
		bool first = true;
		double x = hitsize/2.0;
		double inc = double(width) / data.data.beats.size();

		set_mark_pen();
		for(auto const& beat : data.data.beats) {
			if(!beat.division.empty()) {
				draw_split(x, inc, first, beat.division);
				set_mark_pen();
			} else {
				draw_mark(x, first, beat.action);
			}
			first = false;
			x += inc;
		}
	}

	void draw_default_marks() {
		painter.setPen(mark_pen);
		bool first = true;
		double x = hitsize/2.0;
		double inc = double(width) / data.default_division;
		for(int i = 0; i != data.default_division; ++i) {
			draw_mark(x, first, beat::none);
			first = false;
			x += inc;
		}
	}

	QPainter& painter;
	bar_data const& data;
	QPen mark_pen;
	QPen black_pen;
	QPen red_pen;
	QPen blue_pen;
	QBrush brush;
	QBrush red_brush;

	int hitsize{};
	int current_beat{};

	int const width = painter.device()->width();
	int const height = painter.device()->height();
	int const accent_mark_height = (2*height) / 3;
	int const accent_mark_y_start = (height - accent_mark_height) / 2;
	int const mark_height = height / 3;
};
}

void bar_item::paint(QPainter* painter) {
	drawer d(painter, data_);
	d.draw();
}

}

