// d3.json("./data/jsontree/jsontree_ply_2.json").then(data => {
//   console.log("Json Data", data);
// });

// The size settings
// TODO: Size and position the board container better
const margin = {top: 20, right: 90, bottom: 30, left: 90},
      width = 900,
      height = 900,
      _width = width - margin.left - margin.right,
      _height = height - margin.top - margin.bottom;

// The animation duration
const duration = 750;

// The node index
const i = 0;

// The layout algorithm
var tree = d3.tree()
             .size([_height, _width]);

// The element containing the d3 graph
const svg = d3.select("#svgContainer").append("svg")
            .attr("viewBox", [0, 0, width, height])
            .attr("width", width)
            .attr("height", height)
            .append("g")
            .attr("transform", "translate("
                  + margin.left + "," + margin.top + ")");

const nodeRadius = 8;


// function score(d) {
//   let ret = d.total / (d.visits + 1);
//   return d.id & 1 ? 1.0 - ret : ret;
// }
function score(d) {
  return ret = d.total / d.visits;
}

// Create the div showing the current board
const boardContainer = d3.select("#boardContainer");

var navHistory = [];
var root;

var gameTurn = 0
draw();

function draw(t=gameTurn) {
  let json_fp = `./data/jsontree/jsontree_ply_${t * 2}.json`;

  d3.json(json_fp).then(data => {

    sortEachChildren(data);

    navHistory.push(data);
    updateBoard();

    // d3.hierarchy here stores the @height (max distance to a leaf) and @depth
    // of each nodes.
    root = d3.hierarchy(data);

    console.log("root: ", root);

    // (x0, y0) represent the 'old' position of a source node during an update
    root.x0 = height / 2;
    root.y0 = 0;

    // Collapse everything beyond depth 1 and draw what's left
    root.children.forEach(collapse);
    update(root);
  });
}

function update(source) {
  // The layout object is the same as root but with the added data
  // of an 'x' and a 'y' coordinate for each nodes
  layout = tree(root);//.sort(function(a, b) { return score(b) - score(a); });

  updateBoard();

  // NOTE: @layout contains all the nodes of the tree, but layout.descendants()
  // returns only the nodes that are currently non-collapsed, since the other ones
  // have their @children array swapped with an auxiliary @_children object.
  const nodes = root.descendants(),
        links = root.links();

  if (source.height < width / 180) {
    // Normalize for fixed-depth
    nodes.forEach(d => {
      d.y = d.depth * 180
    });
  }

  function linkColor(link) {
    const edge_weight = d3.scaleDiverging(t => d3.interpolateRdBu(t));
                          //.domain([0, 0.5, 1.0]); This is the default value
    return edge_weight(score(link.source.data));
  }

  // The 'g.node' elements are the representatives of our in-memory nodes
  // that will live on the DOM.
  // The @data() function BINDS the data of our array of json objects (i.e. @nodes)
  // to "virtual" DOM-elements under the 'g' tag in the svg. They are virtual in the
  // sense that they only live as a data-structure in memory until they are actually
  // needed. At that point, they are created according to the "ENTER selection".
  //
  // In the ENTER selection, actual DOM elements are created, then the UPDATE selection
  // specifies how to smoothly transition those new elements from nothing to nodes positionned
  // at/of the size of the corresponding nodes in @layout.
  //
  // TODO: Could think about passing all of @data to the join, and only then
  // collapsing for the first call?
  // NOTE: It seems that the 'key' function used assumes that the 'id' field
  // is initially empty?
  // Maybe try function(d, i) { return d ? d.id : this.id; }
  const node = svg.selectAll("g.node")
                  .data(nodes);
  
  // Enters a node in the DOM. The translation function is there so that
  // any entering node is initially positionned at the source of the click
  // (Entering nodes are modeling the 'expansion' of a collapsed node).
  const nodeEnter = node.enter().append("g")
                        .attr('class', "node")
                        .attr("transform", function(d) {
                          return "translate(" + source.y0 + "," + source.x0 + ")";
                        })
                        .on("click", click);

  console.log("Node Enter selection: ", nodeEnter);

  // Add circles for the entering nodes
  // Initially, the circles are tiny for entering nodes.
  nodeEnter.append('circle')
           .attr('class', 'node')
           .attr('r', 1e-6)
           .style("fill", function(d) {
             return d._children ? "lightsteelblue" : "#fff";
           });

  // Add the entering labels
  nodeEnter.append('text')
           .attr("dy", ".35em")
           .attr("x", function(d) {
             return d.children ? -nodeRadius - 1 : nodeRadius + 1;
           })
           .attr("text-anchor", function(d) {
             return d.children ? "end" : "start";
           })
           .text(function(d) { return name(d.data, d); })
           .style("fill-opacity", 1e-6);

  // The UPDATE selection (the changes are applied to both the entering
  // nodes and those that were already in the DOM)
  const nodeUpdate = nodeEnter.merge(node);

  nodeUpdate.transition()
            .duration(duration)
            .attr("transform", function(d) {
              return "translate(" + d.y + "," + d.x + ")";
            });

  // TODO: Explore setting the circle size proportional to the node's value here
  // Leave the nodes which cannot be further expanded with an empty white circle
  nodeUpdate.select('circle.node')
            .attr('r', nodeRadius)  // .attr('r', function(d){ return edge_weight(d.size/2); })
            .style("fill", function(d) {
              return d._children ? "lightsteelblue" : "#fff";
            })
            .attr('cursor', 'pointer');

  nodeUpdate.select("text")
            .style("fill-opacity", 1);

  // Remove any exiting nodes. Transition them towards the
  // current node's new position so that it looks like they
  // are "absorbed" at the node where we clicked.
  var nodeExit = node.exit().transition()
                     .duration(duration)
                     .attr("transform", function(d) {
                       return "translate(" + source.y + "," + source.x + ")";
                     })
                     .remove();

  // NOTE: The transition time is not quite uniform for all of the Exit selelection.
  // By making the following attributes 0 we make it look like they really disappear
  // as soon as the animation is over.
  nodeExit.select('circle')
          .attr('r', 1e-6);

  nodeExit.select('text')
          .style('fill-opacity', 1e-6);

    // ****************** links section ***************************

  var link = svg.selectAll('path.link')
                .data(links);
  //TODO No need to declare new variables for those??
  // Enter any new links at the current node's previous position.
  var linkEnter = link.enter().insert('path', "g")
                      .attr("class", "link")
                      .attr("stroke-width", function(d) {
                        return 1;
                      })
                      .attr('d', function(d) {
                        var s = {x: source.x0, y: source.y0}
                        return diagonal(s, s);               // diagonal only needs the x and y coordinates
                      })
                      .attr("stroke", function(d) {
                        return linkColor(d);
                      });

  // UPDATE (Is this not needed again?)
  var linkUpdate = linkEnter.merge(link);

  // Transition back to the parent element position
  linkUpdate.transition()
            .duration(duration)
            .attr("d", function(d) {
              return diagonal(d.source, d.target);
            });
            // .attr("stroke-width", function(d) {
            //   return edge_weight(d.target.size);
            // });
            // .attr('d', function(d){ return diagonal(d, d.parent) });

  // Remove the exiting links after transitioning them to the
  // current node's new position
  var linkExit = link.exit().transition()
                     .duration(duration)
                     .attr('d', function(d) {
                       //var s = {x: source.x0, y: source.y0};
                       return diagonal(d.source, d.source);
                     })
                     .remove();

  // Store the current positions so that we have a previous
  // (x0, y0) for the next update.
  nodes.forEach(d => {
    d.x0 = d.x;
    d.y0 = d.y;
  });

  function name(d) {
    return `${d.name}: ${d3.format(".2f")(d.total / (d.visits + 1))}`;
  }

  function diagonal(s, d) {
    path = `M ${s.y} ${s.x}
                    C ${(s.y + d.y) / 2} ${s.x},
                    ${(s.y + d.y) / 2} ${d.x},
                    ${d.y} ${d.x}`
    return path;
  }
}

// Toggle children on click and update the board display
function click(event, d) {

  if (d.children) {

    d._children = d.children;
    d.children = null;

    // Undo the move in the navigation stack unless root
    if (d != root)
      navHistory.pop();

  } else {

    // Apply the move in the navigation stack unless no children
    if (!d._children)
      return;

    d.children = d._children;
    d._children = null;

    navHistory.push(d.data);
  }

  // Update the boardContainer div
  updateBoard();

  // Update the tree
  update(d);
}


d3.select("#prevPly").on("click", function(event) {
  if (gameTurn > 0) {
    --gameTurn;
    collapse(root);
    draw(gameTurn);
  }
});

d3.select("#nextPly").on("click", function(event) {
  if (gameTurn < 55) {
    ++gameTurn;
    collapse(root);
    draw(gameTurn);
  }
});


// Collapse the node and all it's children
function collapse(d) {
  if(d.children) {
    d._children = d.children
    d._children.forEach(collapse)
    d.children = null
  }
}

// Expand the node and all it's children
function expand(d) {
  if (d._children) {
    d.children = d._children;
    d._children = null;
  }
  if (d.children) {
    d.children.forEach(expand);
  }
}

// Expands all the nodes in the tree
function expandAll() {
  root.children.forEach(expand);
  update(root);
}

function updateBoard() {
  const s = navHistory[navHistory.length - 1].str;
  const pboard = boardContainer.select(".boardStr")
                               .text(s);
}


function sortEachChildren(d) {
  if (d.children.length < 2)
    return;

  d.children.sort(function(a, b) { return score(b) - score(a); });

  for (let i = 0; i < d.children.length; ++i) {
    sortEachChildren(d.children[i]);
  }
}


// Find the child object in the json structure corresponding to the link's target
// in order to get the position in y of the source
// function caculateLinkSourcePosition(link) {
//   targetId = link.target.id;
//   let childrenNumber = link.source.children.length;
//   let widthAbove = 0;
//   for (const i=0; i<childrenNumber; ++i) {
//     if (link.source.children[i].id == targetId) {
//       widthAbove = widthAbove + link.source.children[i].size/2;
//       break;
//     }
//     widthAbove += link.source.children[i].size
//   }
//   return link.source.size/2 - widthAbove;
// }


// function Tree(data, {
//   link, // given a node d, its link (if any)
//   name = function(d) { return `${d.name}: ${d3.format(".2f")(d.total / (d.visits + 1))}`; }, // given a node d, its display name
//   str = function(d) { return d.str; }, // given a node d, a representation of the board
//   score = function(d) {
//     let v = d.total / (d.visits + 1);
//     return (d.ply & 1 ? 1.0 - v : v); },
//   navHistory, // to save the navigation history
//   width = 960, // outer width, in pixels
//   height = 1040, // outer height, in pixels
//   dx = 10, // the vertical size of a node
//   dy, // the horizontal size of each nodes
//   const margin = {top: 20, right: 90, bottom: 30, left: 90},
//   r = 8, // radius of nodes
//   fill = "#fff", // fill for nodes
//   fillOpacity, // fill opacity for nodes
//   stroke = "#ccc", // stroke for links
//   strokeWidth = 2, // stroke width for links
//   strokeOpacity = 0.4, // stroke opacity for links
//   strokeLinejoin, // stroke line join for links
//   strokeLinecap, // stroke line cap for links
//   halo = "#fff", // color of label halo
//   haloWidth = 3, // padding around the labels
//   duration = 750, // the duration of the clickEvent transitions in ms
//   } = {}) {

//   // // The size of the tree content
//   // let _width = width - margin.left - margin.right,
//   //     _height = height - margin.top - margin.bottom;

//   // //the layout algorithm
//   // const tree = d3.tree().size([_height, _width]);

//   // // the d3 data-structure for the tree {parent, children, height, depth} are assigned
//   // const root = d3.hierarchy(data);

//   // // Position the root
//   // root.x0 = height / 2;
//   // root.y0 = 0;

//   // // Create the svg containing the tree and set its width/height according
//   // // to the above
//   // const svg = d3.select("body").append("svg")
//   //               .attr("viewBox", [0, 0, width, height])
//   //               .attr("width", width)
//   //               .attr("height", height)
//   //               .append("g")
//   //               .attr("transform", "translate("
//   //                     + margin.left + "," + margin.top + ")");

//   // // Create the div showing the current board
//   // const boardContainer = d3.select("#boardContainer");
//   // //const boardContainerCode = d3.select("#boardContainer").select("code");
//   // // Initialize the navigation stack and load the root's board in
//   // // the boardContainer div
//   // navHistory = [root.data];
//   // updateBoard();

//   // // Collapse the tree below depth 1
//   // root.children.forEach(collapse);
//   // maxVisibleDepth = root.children ? 1 : 0;

//   // update(root);

//   // Collapse the node and all it's children
//   // function collapse(d) {
//   //   if(d.children) {
//   //     d._children = d.children
//   //     d._children.forEach(collapse)
//   //     d.children = null
//   //   }
//   // }

//   // function updateBoard() {
//   //   const s = str(navHistory[navHistory.length - 1])
//   //   const pboard = boardContainer.select(".boardStr")
//   //                                .text(s);
//   // }

//   function update(source) {
//     layout = tree(root);

//     layout.sort((a, b) => d3.descending(score(a), score(b)));

//     // Compute the new layout (after expanding/collapsing)
//     let nodes = layout.descendants(),
//         links = layout.descendants().slice(1);

//     // Normalize for fixed-depth
//     nodes.forEach(d => {
//       d.y = d.depth * 180
//     });

//     // Update the nodes
//     let node = svg.selectAll("g.node")
//                   .data(nodes, function(d, i) { return d.id || (d.id = ++i); } );

//     // // On click,
//     let nodeEnter = node.enter().append("g")
//                         .attr('class', "node")
//                         .attr("transform", function(d) {
//                           return "translate(" + source.y0 + "," + source.x0 + ")";
//                         })
//                         .on("click", click);

//     // Add circles for the entering nodes
//     nodeEnter.append('circle')
//              .attr('class', 'node')
//              .attr('r', 1e-6)
//              .style("fill", function(d) {
//                return d._children ? "lightsteelblue" : "#fff";
//              });

//     // Add labels for the entering nodes
//     nodeEnter.append('text')
//              .attr("dy", ".35em")
//              .attr("x", function(d) {
//                return d.children || d._children ? -13 : 13;
//              })
//              .attr("text-anchor", function(d) {
//                return d.children || d._children ? "end" : "start";
//              })
//              .text(function(d) { return name(d.data, d); });

//     let nodeUpdate = nodeEnter.merge(node);

//     // Transition to the proper position for the node
//     nodeUpdate.transition()
//               .duration(duration)
//               .attr("transform", function(d) {
//                 return "translate(" + d.y + "," + d.x + ")";
//               });

//     // Update the node attributes, style and hover text
//     nodeUpdate.select('circle.node')
//               .attr('r', r)
//               .style("fill", function(d) {
//                 return d._children ? "lightsteelblue" : "#fff";
//               })
//               .attr('cursor', 'pointer');

//     // Remove any exiting nodes
//     var nodeExit = node.exit().transition()
//                        .duration(duration)
//                        .attr("transform", function(d) {
//                          return "translate(" + source.y + "," + source.x + ")";
//                        })
//                        .remove();

//     // On exit reduce the node circles size to 0
//     nodeExit.select('circle')
//             .attr('r', 1e-6);

//     // On exit reduce the opacity of text labels
//     nodeExit.select('text')
//             .style('fill-opacity', 1e-6);

//     // ****************** links section ***************************

//     // Update the links...
//     var link = svg.selectAll('path.link')
//                   .data(links, function(d) { return d.id; });

//     // Enter any new links at the parent's previous position.
//     var linkEnter = link.enter().insert('path', "g")
//                         .attr("class", "link")
//                         .attr('d', function(d){
//                           var o = {x: source.x0, y: source.y0}
//                           return diagonal(o, o)
//                         });

//     // UPDATE
//     var linkUpdate = linkEnter.merge(link);

//     // Transition back to the parent element position
//     linkUpdate.transition()
//               .duration(duration)
//               .attr('d', function(d){ return diagonal(d, d.parent) });

//     // Remove any exiting links
//     var linkExit = link.exit().transition()
//                        .duration(duration)
//                        .attr('d', function(d) {
//                          var o = {x: source.x, y: source.y}
//                          return diagonal(o, o)
//                        })
//                        .remove();

//     // Store the old positions for transition.
//     nodes.forEach(function(d){
//       d.x0 = d.x;
//       d.y0 = d.y;
//     });

//     function diagonal(s, d) {
//       path = `M ${s.y} ${s.x}
//                     C ${(s.y + d.y) / 2} ${s.x},
//                     ${(s.y + d.y) / 2} ${d.x},
//                     ${d.y} ${d.x}`
//       return path
//     }

//     // const diagonal = d3.svg.diagonal()
//     //                .projection(function(d) { return [d.y, d.x]; });

//     // Toggle children on click.
//     function click(event, d) {
//       if (d.children) {
//         d._children = d.children;
//         d.children = null;

//         // Undo the move in the navigation stack
//         navHistory.pop();
//       } else {
//         d.children = d._children;
//         d._children = null;

//         // Apply the move in the navigation stack
//         navHistory.push(d.data);
//       }

//       // Update the boardContainer div
//       updateBoard();

//       // Update the tree
//       update(d);
//     }
//   }
// }
