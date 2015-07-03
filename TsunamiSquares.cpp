// Copyright (c) 2015 Kasey W. Schultz, Eric M. Heien, Steven N. Ward
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "TsunamiSquares.h"

// ----------------------------------------------------------------------
// -------------------- Square Functions --------------------------------
// ----------------------------------------------------------------------
tsunamisquares::SquareIDSet tsunamisquares::ModelWorld::getSquareIDs(void) const {
    SquareIDSet square_id_set;
    std::map<UIndex, Square>::const_iterator  sit;

    for (sit=_squares.begin(); sit!=_squares.end(); ++sit) {
        square_id_set.insert(sit->second.id());
    }

    return square_id_set;
}

tsunamisquares::SquareIDSet tsunamisquares::ModelWorld::getVertexIDs(void) const {
    SquareIDSet vertex_id_set;
    std::map<UIndex, Vertex>::const_iterator  vit;

    for (vit=_vertices.begin(); vit!=_vertices.end(); ++vit) {
        vertex_id_set.insert(vit->second.id());
    }

    return vertex_id_set;
}

std::map<double, tsunamisquares::UIndex> tsunamisquares::ModelWorld::getNeighborIDs(const tsunamisquares::Vec<2> &location) const {
    std::map<double, UIndex>                  square_dists, neighbors;
    std::map<double, UIndex>::const_iterator  it;
    std::map<UIndex, Square>::const_iterator  sit;

    // Compute distance from "location" to the center of each square.
    // Since we use a map, the distances will be ordered since they are the keys
    for (sit=_squares.begin(); sit!=_squares.end(); ++sit) {
        double square_dist = sit->second.center().dist(location);
        square_dists.insert(std::make_pair(square_dist, sit->second.id()));
    }
    
    // Grab the closest 4 squares and return their IDs
    for (it=square_dists.begin(); it!=square_dists.end(); ++it) {
        double dist = it->first;
        UIndex id = it->second;
        neighbors.insert(std::make_pair(dist, id));
        if (neighbors.size() == 4) break;
    }
    
    return neighbors;
}

// Set the height for all elements equal to the depth of the bathymetry below the center of the square.
// Result is squares with just enough water so that the water sits at sea level.
void tsunamisquares::ModelWorld::fillToSeaLevel(void) {
    std::map<UIndex, Square>::iterator     it;

    for (it=_squares.begin(); it!=_squares.end(); ++it) {
        it->second.set_height(it->second.center_depth());
    }
}


//// Move the water from a Square given its current velocity and acceleration.
//// Partition the volume and momentum into the neighboring Squares.
//void tsunamisquares::ModelWorld::moveSquare(const UIndex &square_id, const double dt) {
//    Square this_square = this->square(square_id);
//    Vec<2> current_velo, current_accel, current_pos, new_pos, new_velo;
//    std::map<double, UIndex> neighbor_dists;
//    std::map<double, UIndex>::const_iterator dit;
//    
//    current_pos = this_square.center();
//    current_velo = this_square.velocity();
//    current_accel = this_square.accel();
//    
//    new_pos = current_pos + dt*current_velo + 0.5*dt*dt*current_accel;
//    new_velo = current_velo + dt*current_accel;
//    
//    // Compute
//    for (dit=neighbor_dists.begin(); dit!=neighbor_dists.end(); ++dit){
//        dit->first;
//    }
//    
//    
//    
//}


// ----------------------------------------------------------------------
// -------------------- Model Building/Editing --------------------------
// ----------------------------------------------------------------------
tsunamisquares::Square &tsunamisquares::ModelWorld::square(const UIndex &ind) throw(std::domain_error) {
    std::map<UIndex, Square>::iterator it = _squares.find(ind);

    if (it == _squares.end()) throw std::domain_error("tsunamisquares::ModelWorld::square");
    else return it->second;
}

tsunamisquares::Vertex &tsunamisquares::ModelWorld::vertex(const UIndex &ind) throw(std::domain_error) {
    std::map<UIndex, Vertex>::iterator it = _vertices.find(ind);

    if (it == _vertices.end()) throw std::domain_error("tsunamisquares::ModelWorld::vertex");
    else return it->second;
}

tsunamisquares::Square &tsunamisquares::ModelWorld::new_square(void) {
    UIndex  max_ind = next_square_index();
    _squares.insert(std::make_pair(max_ind, Square()));
    _squares.find(max_ind)->second.set_id(max_ind);
    return _squares.find(max_ind)->second;
}

tsunamisquares::Vertex &tsunamisquares::ModelWorld::new_vertex(void) {
    UIndex  max_ind = next_vertex_index();
    _vertices.insert(std::make_pair(max_ind, Vertex()));
    _vertices.find(max_ind)->second.set_id(max_ind);
    return _vertices.find(max_ind)->second;
}

void tsunamisquares::ModelWorld::clear(void) {
    _squares.clear();
    _vertices.clear();
}

void tsunamisquares::ModelWorld::reset_base_coord(const LatLonDepth &new_base) {
    std::map<UIndex, Vertex>::iterator         it;

    for (it=_vertices.begin(); it!=_vertices.end(); ++it) {
        it->second.set_lld(it->second.lld(), new_base);
    }

    _base = new_base;
}

void tsunamisquares::ModelWorld::insert(tsunamisquares::Square &new_square) {
    // We also want to set the Squares _verts to the (x,y,z) coords of its vertices
    for (unsigned int i=0; i<4; ++i) {
        new_square.set_vert(i, this->vertex(new_square.vertex(i)));
    }
    _squares.insert(std::make_pair(new_square.id(), new_square));
}

void tsunamisquares::ModelWorld::insert(const tsunamisquares::Vertex &new_vertex) {
    _vertices.insert(std::make_pair(new_vertex.id(), new_vertex));
}

size_t tsunamisquares::ModelWorld::num_squares(void) const {
    return _squares.size();
}

size_t tsunamisquares::ModelWorld::num_vertices(void) const {
    return _vertices.size();
}

void tsunamisquares::ModelWorld::printSquare(const UIndex square_id) {
    Square this_square = this->square(square_id);

    std::cout << "~~~ Square " << this_square.id() << "~~~" << std::endl;
    for (unsigned int i=0; i<4; ++i) {
        std::cout << " vertex " << this_square.vertex(i) << ": lld " << this->vertex(this_square.vertex(i)).lld() << std::endl;
    }
    std::cout << "center: " << this_square.center() << std::endl;
    std::cout << "density: " << this_square.density() << std::endl;
    std::cout << "area: " << this_square.area() << std::endl;
    if (!isnan(this_square.height())) {
        std::cout << "height: " << this_square.height() << std::endl;
        std::cout << "volume: " << this_square.volume() << std::endl;
        std::cout << "mass: " << this_square.mass() << std::endl;
        std::cout << "velocity: " << this_square.velocity() << std::endl; 
        std::cout << "accel: " << this_square.accel() << std::endl;    
        std::cout << "momentum: " << this_square.momentum() << std::endl; 
    }
}

void tsunamisquares::ModelWorld::printVertex(const UIndex vertex_id) {
    Vertex this_vert = this->vertex(vertex_id);
    std::cout << " ~ Vertex " << this_vert.id() << "~" << std::endl;
    std::cout << "position(xyz): " << this_vert.xyz() << std::endl; 
    std::cout << "position(lld): " << this_vert.lld() << std::endl; 
}

void tsunamisquares::ModelWorld::info(void) const{
    std::cout << "ModelWorld: " << this->num_squares() << " squares, " << this->num_vertices() << " vertices." << std::endl;
}

void tsunamisquares::ModelWorld::get_bounds(LatLonDepth &minimum, LatLonDepth &maximum) const {
    std::map<UIndex, Vertex>::const_iterator    it;
    double      min_lat, min_lon, min_alt;
    double      max_lat, max_lon, max_alt;

    min_lat = min_lon = min_alt = DBL_MAX;
    max_lat = max_lon = max_alt = -DBL_MAX;

    for (it=_vertices.begin(); it!=_vertices.end(); ++it) {
        min_lat = fmin(min_lat, it->second.lld().lat());
        max_lat = fmax(max_lat, it->second.lld().lat());
        min_lon = fmin(min_lon, it->second.lld().lon());
        max_lon = fmax(max_lon, it->second.lld().lon());
        min_alt = fmin(min_alt, it->second.lld().altitude());
        max_alt = fmax(max_alt, it->second.lld().altitude());
    }

    if (min_lat == DBL_MAX || min_lon == DBL_MAX || min_alt == DBL_MAX) {
        minimum = LatLonDepth();
    } else {
        minimum = LatLonDepth(min_lat, min_lon, min_alt);
    }

    if (max_lat == -DBL_MAX || max_lon == -DBL_MAX || max_alt == -DBL_MAX) {
        maximum = LatLonDepth();
    } else {
        maximum = LatLonDepth(max_lat, max_lon, max_alt);
    }
}


// ----------------------------------------------------------------------
// ----------------------------- Model File I/O -------------------------
// ----------------------------------------------------------------------
std::string tsunamisquares::ModelIO::next_line(std::istream &in_stream) {
    std::string line = "";
    size_t      pos;

    do {
        std::getline(in_stream, line);
        _comment = "";
        // Cut off any initial whitespace
        pos = line.find_first_not_of(" \t");

        if (pos != std::string::npos) line = line.substr(pos, std::string::npos);

        // Comment consists of hash mark until the end of the line
        pos = line.find("#");

        if (pos != std::string::npos) _comment = line.substr(pos, std::string::npos);

        // Extract the non-comment part of the line
        line = line.substr(0, line.find("#"));

        // If the line is empty, we keep going
        if (line.length() > 0) break;
    } while (in_stream && !in_stream.eof());

    return line;
}

void tsunamisquares::ModelIO::next_line(std::ostream &out_stream) const {
    if (!_comment.empty()) out_stream << " # " << _comment;

    out_stream << "\n";
}

void tsunamisquares::Square::get_field_descs(std::vector<FieldDesc> &descs) {
    FieldDesc       field_desc;

    field_desc.name = "id";
    field_desc.details = "Unique ID of the square.";
    descs.push_back(field_desc);

    field_desc.name = "vertex_0";
    field_desc.details = "ID of vertex 0.";
    descs.push_back(field_desc);

    field_desc.name = "vertex_1";
    field_desc.details = "ID of vertex 1.";
    descs.push_back(field_desc);

    field_desc.name = "vertex_2";
    field_desc.details = "ID of vertex 2.";
    descs.push_back(field_desc);
    
    field_desc.name = "vertex_3";
    field_desc.details = "ID of vertex 3.";
    descs.push_back(field_desc);

    field_desc.name = "friction";
    field_desc.details = "Coefficient of friction for acceleration.";
    descs.push_back(field_desc);

//    field_desc.name = "is_boundary";
//    field_desc.details = "Whether or not the square is along the boundary.";
//    descs.push_back(field_desc);

}

void tsunamisquares::Square::read_data(const SquareData &in_data) {
    memcpy(&_data, &in_data, sizeof(SquareData));
}

void tsunamisquares::Square::write_data(SquareData &out_data) const {
    memcpy(&out_data, &_data, sizeof(SquareData));
}

void tsunamisquares::Square::read_ascii(std::istream &in_stream) {
    unsigned int        i;
    std::stringstream   ss(next_line(in_stream));

    ss >> _data._id;

    for (i=0; i<4; ++i) ss >> _data._vertices[i];

    ss >> _data._friction;
    //ss >> _data._is_boundary;
}

void tsunamisquares::Square::write_ascii(std::ostream &out_stream) const {
    unsigned int        i;

    out_stream << _data._id << " ";

    for (i=0; i<4; ++i) out_stream << _data._vertices[i] << " ";

    out_stream << _data._friction << " ";
    //out_stream << _data._is_boundary << " ";

    next_line(out_stream);
}

void tsunamisquares::Vertex::get_field_descs(std::vector<FieldDesc> &descs) {
    FieldDesc       field_desc;

    field_desc.name = "id";
    field_desc.details = "Unique ID of the vertex.";
    descs.push_back(field_desc);

    field_desc.name = "latitude";
    field_desc.details = "Latitude of the vertex.";
    descs.push_back(field_desc);

    field_desc.name = "longitude";
    field_desc.details = "Longitude of the vertex.";
    descs.push_back(field_desc);

    field_desc.name = "altitude";
    field_desc.details = "Altitude of the vertex in meters (negative is below ground).";
    descs.push_back(field_desc);

}

void tsunamisquares::Vertex::read_data(const VertexData &in_data) {
    memcpy(&_data, &in_data, sizeof(VertexData));
}

void tsunamisquares::Vertex::write_data(VertexData &out_data) const {
    memcpy(&out_data, &_data, sizeof(VertexData));
}

void tsunamisquares::Vertex::read_ascii(std::istream &in_stream) {
    std::stringstream   ss(next_line(in_stream));

    ss >> _data._id;
    ss >> _data._lat;
    ss >> _data._lon;
    ss >> _data._alt;
}

void tsunamisquares::Vertex::write_ascii(std::ostream &out_stream) const {
    out_stream << _data._id << " ";
    out_stream << _data._lat << " ";
    out_stream << _data._lon << " ";
    out_stream << _data._alt << " ";
    next_line(out_stream);
}

int tsunamisquares::ModelWorld::read_file_ascii(const std::string &file_name) {
    std::ifstream       in_file;
    unsigned int        i, j, num_squares, num_vertices;
    LatLonDepth         min_latlon, max_latlon;

    // Clear the world first to avoid incorrectly mixing indices
    clear();

    in_file.open(file_name.c_str());

    if (!in_file.is_open()) return -1;

    // Read the first line describing the number of sections, etc
    std::stringstream desc_line(next_line(in_file));
    desc_line >> num_squares;
    desc_line >> num_vertices;

    // Read elements
    for (i=0; i<num_squares; ++i) {
        Square     new_square;
        new_square.read_ascii(in_file);
        _squares.insert(std::make_pair(new_square.id(), new_square));
    }

    // Read vertices
    for (i=0; i<num_vertices; ++i) {
        Vertex     new_vert;
        new_vert.read_ascii(in_file);
        _vertices.insert(std::make_pair(new_vert.id(), new_vert));
    }

    in_file.close();
    
    
    // Reset the internal Cartesian coordinate system
    get_bounds(min_latlon, max_latlon);
    min_latlon.set_altitude(0);
    reset_base_coord(min_latlon);
    // Keep track of Lat/Lon bounds in the ModelWorld
    _min_lat = min_latlon.lat();
    _min_lon = min_latlon.lon();
    _max_lat = max_latlon.lat();
    _max_lon = max_latlon.lon();
    
    // Manually assign vertex xyz coords for the vertices given the new base lld
    for (i=0; i<num_vertices; ++i) {
        _vertices[i].set_lld(_vertices[i].lld(), getBase());
    }
    
    // Manually assign vertex xyz coords for the vertices in the square objects
    for (i=0; i<num_squares; ++i) {
        for (j=0; j<4; ++j) {
            _squares[i].set_vert(j, _vertices[ _squares[i].vertex(j) ]);
        }
    }
    

    return 0;
}

int tsunamisquares::ModelWorld::write_file_ascii(const std::string &file_name) const {
    std::ofstream                                   out_file;
    std::vector<FieldDesc>                          descs;
    std::vector<FieldDesc>::iterator                dit;
    std::map<UIndex, Vertex>::const_iterator   vit;
    std::map<UIndex, Square>::const_iterator  eit;

    out_file.open(file_name.c_str());
    out_file << "# Number of squares\n";
    out_file << "# Number of vertices\n";
    out_file << " " << _squares.size() << " " << _vertices.size();
    next_line(out_file);

    // Write element header
    descs.clear();
    Square::get_field_descs(descs);

    for (dit=descs.begin(); dit!=descs.end(); ++dit) {
        out_file << "# " << dit->name << ": " << dit->details << "\n";
    }

    out_file << "# ";

    for (dit=descs.begin(); dit!=descs.end(); ++dit) {
        out_file << dit->name << " ";
    }

    out_file << "\n";

    // Write squares
    for (eit=_squares.begin(); eit!=_squares.end(); ++eit) {
        eit->second.write_ascii(out_file);
    }

    // Write vertex header
    descs.clear();
    Vertex::get_field_descs(descs);

    for (dit=descs.begin(); dit!=descs.end(); ++dit) {
        out_file << "# " << dit->name << ": " << dit->details << "\n";
    }

    out_file << "# ";

    for (dit=descs.begin(); dit!=descs.end(); ++dit) {
        out_file << dit->name << " ";
    }

    out_file << "\n";

    // Write vertices
    for (vit=_vertices.begin(); vit!=_vertices.end(); ++vit) {
        vit->second.write_ascii(out_file);
    }

    out_file.close();

    return 0;
}