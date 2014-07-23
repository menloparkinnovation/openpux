
//
// This allows the listed functions to be available from this
// module.
//
module.exports = {
  helloFunc: function () {
        var args = process.argv.slice(2);
        console.log("helloFunc: arg:");
	console.log(args);
  }
};
