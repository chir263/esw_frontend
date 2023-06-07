window.onscroll = function () {
  scrollFunction();
};

function findTop(element) {
  var rec = element.getBoundingClientRect();
  return rec.top + window.scrollY;
}

var aa = findTop(document.getElementById("navbar"));

function scrollFunction() {
  if (document.body.scrollTop > aa || document.documentElement.scrollTop > aa) {
    document.getElementById("navbar").style.position = "fixed";
    document.getElementById("navbar").style.top = "0px";
  } else {
    document.getElementById("navbar").style.position = "relative";
  }
}
