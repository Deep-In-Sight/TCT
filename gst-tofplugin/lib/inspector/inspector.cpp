/* Copyright (C) 2023 Deep In Sight
 * Author: Le Ngoc Linh <lnlinh93@dinsight.ai>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION: inspector
 *
 * Add description later
 *
 */
#include <lib/inspector/inspector.h>

Inspector::Inspector(const std::string name) { ; }

Inspector::~Inspector() { ; }

int Inspector::attach(GstPad* pad) { return -1; }

void Inspector::detach() { ; }

int Inspector::add_subscriber(InspectorClient* client) { return -1; }

size_t Inspector::get_num_samples() { return -1; }