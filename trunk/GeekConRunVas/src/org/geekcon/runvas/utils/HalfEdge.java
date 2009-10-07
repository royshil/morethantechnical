package org.geekcon.runvas.utils;


public class HalfEdge {
	public HalfEdge next;
	public HalfEdge prev;
	public HalfEdge pair;
	
	public Edge edge;
//	public VertexData origin;
//	FaceData face;
	
//	public Edge addEdge(Vertex vertexFrom, Vertex vertexTo) {
	public static void calcHalfEdges(Vertex vertexFrom, Vertex vertexTo) {
		//check for loop edge
//		if (vertexFrom == vertexTo) {
//			return;
//		}
		
		//check if this edge is already exist
		if (vertexFrom.isIsolated() == false && vertexTo.isIsolated() == false) {
			Edge edge = vertexFrom.vertex.half.edge;
			if (edge.a == vertexFrom && edge.b == vertexTo ||
			    edge.a == vertexTo   && edge.b == vertexFrom) {
				return;
			}
			edge = vertexTo.vertex.half.edge;
			if (edge.a == vertexFrom && edge.b == vertexTo ||
				edge.a == vertexTo   && edge.b == vertexFrom) {
					return;
				}
		}
		//Allocate Date
		Edge edge = new Edge(vertexFrom, vertexTo);
		HalfEdge fromToHalf = new HalfEdge();
		HalfEdge toFromHalf = new HalfEdge();
		
		//initialize data
//		EdgeData edgeData = edge.edge;
//		edgeData.half = fromToHalf;
		
		HalfEdge halfData = fromToHalf;
		halfData.next = toFromHalf;
		halfData.prev = toFromHalf;
		halfData.pair = toFromHalf;
//		halfData.origin = vertexFrom.vertex;
		halfData.edge = edge;
//		halfData.face = null;
		
		halfData = toFromHalf;
		halfData.next = fromToHalf;
		halfData.prev = fromToHalf;
		halfData.pair = fromToHalf;
//		halfData.origin = vertexTo.vertex;
		halfData.edge = edge;
//		halfData.face = null;
		
		//Link the from side of the edge
		if (vertexFrom.isIsolated() == true) { 
			vertexFrom.vertex.half = fromToHalf;
		}
		else {
			HalfEdge fromIn = findFreeIncident(vertexFrom);
			HalfEdge fromOut = fromIn.next;
			
			fromIn.next = fromToHalf;
			fromToHalf.prev = fromIn;
			
			toFromHalf.next = fromOut;
			fromOut.prev = toFromHalf;
		}
		
		//Link the to side of the edge
		if (vertexTo.isIsolated() == true) {
			vertexTo.vertex.half = toFromHalf;
		}
		else {
			HalfEdge toIn = findFreeIncident(vertexTo);
			HalfEdge toOut = toIn.next;
			
			toIn.next = toFromHalf;
			toFromHalf.prev = toIn;
			
			fromToHalf.next = toOut;
			toOut.prev = fromToHalf;
		}
		
//		return edge;
	}
	
	private static HalfEdge findFreeIncident(Vertex vertex) {
		return vertex.vertex.half.pair;
	}
}
