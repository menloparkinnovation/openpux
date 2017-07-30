
//
// Parse Name=Value into obj.name, obj.value
//
function parseNameValue(str) {

    var position = str.indexOf('=');
    if (position == (-1)) {
        return null;
    }

    var name = str.substring(0, position);
    var value = str.substring(position + 1);

    var nv = new Object();
    nv.name = name;
    nv.value = value;

    return nv;
}
