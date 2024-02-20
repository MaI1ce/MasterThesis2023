(function(){const e=document.getElementById("data-embedded").dataset.referer
function t(t){parent&&parent.postMessage(t,e)}function o(e){let o=e.target.getAttribute("data-link-to-post")
if(o){let n=document.getElementById("post-"+o)
if(n){let o=n.getBoundingClientRect()
if(o&&o.top)return t({type:"discourse-scroll",top:o.top}),e.preventDefault(),!1}}}window.onload=function(){let e=document.querySelector("[data-embed-state]"),n="unknown",l=null
e&&(n=e.getAttribute("data-embed-state"),l=e.getAttribute("data-embed-id")),t({type:"discourse-resize",height:document.body.offsetHeight,state:n,embedId:l})
let r,a=document.querySelectorAll("a[data-link-to-post]")
for(r=0;r<a.length;r++)a[r].onclick=o
let d=document.querySelectorAll(".cooked a")
for(r=0;r<d.length;r++)d[r].target="_blank"
let c=document.querySelectorAll(".username a")
for(r=0;r<c.length;r++){let e=c[r].innerHTML
e&&(c[r].innerHTML=new BreakString(e).break())}let u=document.querySelectorAll(".cooked a.hashtag-cooked")
for(r=0;r<u.length;r++)u[r].querySelector(".hashtag-icon-placeholder .d-icon").remove(),u[r].querySelector(".hashtag-icon-placeholder").innerText="#"}})()

//# sourceMappingURL=embed-application-00e08d311d3285925aeed70aa17bb9b979372a54980d2c65d4ca76e61fb5707b.map
//!
;
