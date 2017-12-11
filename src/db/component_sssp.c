//Joshua Trujillo
//CSCI3287
//Project 3 - grdb Dijkstra's
//Due: 12/10/17

#include "graph.h"
#include <stdio.h>
#include <limits.h>
#include "cli.h"
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

//#define min
//#define INT_MAX_INT

/* Place the code for your Dijkstra implementation in this file */
attribute_t find_int_tuple(attribute_t attr){
	for(attr; attr != NULL; attr = attr->next){
		if(attr->bt == 4){
			return attr;
		}
	}
	return NULL;
}


void file_setup(component_t c, vertexid_t v1){
	//setup
	struct vertex v;
	vertex_init(&v);
	v.id = v1;
	char s[BUFSIZE], *buf;
	
	//get vertex/edge file/descriptor
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/v", grdbdir, gno, cno);
	c->vfd = open(s, O_RDWR);
	
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/e", grdbdir, gno, cno);
	c->efd = open(s, O_RDWR);
}


//work around helper loader function for vertices
void vertices_setup(component_t c, vertexid_t *vertices){
    ssize_t temp, len;
    int r;
    char* buf;
    off_t off;
    vertexid_t v;

    if (c->sv == NULL){
      temp = 0;
    }
    else
    {
      temp = schema_size(c->sv);
    }
    
    len = sizeof(vertexid_t) + temp;
    buf= malloc(len);

    int i=0;
    int x=1;
    while(x)
    {
      lseek(c->vfd, off, SEEK_SET);
      r = read(c->vfd,buf, len);
      if (r <= 0){
          break;
      }  
      
      v = *((vertexid_t *)buf);
      vertices[i] = v;

      off += len;
      i++;
    }
    
}

//find total number of vertices
int num_total_vertices(component_t c) {
	//setup
	off_t off;
	char *buf;
	int num, len, x;
	ssize_t r, temp;
	
	//check
	if(c->sv == NULL) {
		temp = 0;
	}
	else
	{
		temp = schema_size(c->sv);
	}
	
	len = sizeof(vertexid_t) + temp;
	buf = malloc(len);
	
	x=1;
	num=0;
	while(x) 
	{
		lseek(c->vfd, off, SEEK_SET);
		r = read(c->vfd, buf, len);
		if(r == -1 || r == 0) {
			break;
		}
		
		off += len;
		num++;
	}
	
	return num;
}



//helper for finding next shortest path vertex
int connected_vertex(component_t c, vertexid_t v1, vertexid_t *vertices, int t, char *attr) {
	int e, w, i;
	
	e=0;
	for(i = 0; i < t; i++){
		w = tuple_weight(c, v1, vertices[i], attr);
		if(w != INT_MAX){
			e++;
		}
	}
	return e;
}


//
int tuple_weight (component_t c, vertexid_t v1, vertexid_t v2, char* attr) {
	//setup
	struct edge e;
	edge_init(&e);
	edge_t new_edge;
	edge_set_vertices(&e, v1, v2);
	int off, w;
	
	new_edge = component_find_edge_by_ids(c, &e);
	
	if(new_edge == NULL) {
		return INT_MAX;
	}
	
	off = tuple_get_offset(new_edge->tuple, attr);
	w = tuple_get_int(new_edge->tuple->buf + off);
	
	return w;
}


//trying to get this to work...
int component_sssp( component_t c, vertexid_t v1, vertexid_t v2, int *n, int *total_weight, vertexid_t **path) {
	//setup for helping me store with dijkstra's (pretty hairy but it works... mostly!)
	vertexid_t *vertices;
	vertexid_t *temp;
	vertexid_t start_v;
	vertexid_t end_v;
	int *cost;
	
	//setup and get file/descriptors
	file_setup(c, v1);
	
	//total number of vertices
	int total_size = num_total_vertices(c);
	
	char current_path[BUFSIZE];
	char previous[BUFSIZE];
	char *attr;
	
	//allocation nation
	vertices = malloc(total_size * sizeof(vertexid_t));
	temp = malloc(total_size * sizeof(vertexid_t));
	cost = malloc(total_size * sizeof(int));
	
	//loading in
	vertices_setup(c, vertices);
	for(int i = 0; i < total_size; i++){
		cost[i] = INT_MAX;
		temp[i] = v1;
	}
	
	memset(current_path, 0, BUFSIZE);
	sprintf(current_path, "%llu", v1);
 
/*
 * Figure out which attribute in the component edges schema you will
 * use for your weight function
 */

    attribute_t data = find_int_tuple(c->se->attrlist);
    attr = data->name;
    
    vertexid_t current;
    vertexid_t *connected;
    int connected_edges;
    int edge_connections;

/*
 * Execute Dijkstra on the attribute you found for the specified
 * component
 */
 
	//notsure how to get this working and properly access linked, more weird/fun work around helper!
    void help_data_vertices(component_t c, vertexid_t v1, vertexid_t *vertices, int t, char *attr, vertexid_t *e) {
		int temp, i, w;
		
		temp=0;
		for(i=0; i<t; i++) {
			w = tuple_weight(c, v1, vertices[i], attr);
			if(w != INT_MAX) {
				e[temp] = vertices[i];
				temp++;
			}
		}
	}
	
//note-self: if have time think about how to impliment beyond sequential data
//get this working first!!!!

    start_v = v1;
    end_v = v2; 
    
    int w;
    vertexid_t connection;
    
    if(start_v != vertices[0]){
		printf("Input vertex must start at 1.\n");
		return -1;
	}
	else {
		
		//the big bad dijkstra's not too friendly...
		cost[0] = 0;
		for(int i = 0; i < total_size; i++){
			
		  current = vertices[i];
		  
		  int connected_edges = connected_vertex(c, v1, vertices, total_size, attr);
		  connected = malloc(connected_edges *sizeof(vertexid_t));
		  
		  help_data_vertices(c, current, vertices, total_size, attr, connected);

		  int edge_connections = connected_vertex(c, current, vertices,total_size, attr);
		  
		  for(int j = 0; j < edge_connections; j++){
			connection = connected[j];
			w = tuple_weight(c, current, connection, attr);
			
			//check dijkstra alg conditional with C and D
			if(cost[current - 1] + w < cost[connection - 1]){
			  cost[connection - 1] = cost[current - 1] + w;
			  memset(current_path, 0, BUFSIZE);
			  memset(previous, 0, BUFSIZE);
			}
		  }
		}
		
		//not fully sure what is expected but print the cost of shortest
		//note-self: if get working well enough try and maintain a "path"
		//and print it out as well to show...?
		printf("The cost of the specified SSSP is: %i\n", cost[end_v-1]);
		
		return(cost[end_v-1]);
	}
	return -1;
}
