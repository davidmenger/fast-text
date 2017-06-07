# Node.js Fast Text Wrapper

Prediction and nearest neighbour tools from Facebook Fast Text wrapped into Node.js packages.

More here: [Facebook Fast Text](https://github.com/facebookresearch/fastText).

First query takes more time, other queries not :)

## Prediction

There is a simple class for executing prediction models:

```javascript
const path = require('path');
const { Classifier } = require('../main');

const model = path.resolve(__dirname, './classification.bin');

const classifier = new Classifier(model);

classifier.predict('how it works', 1, (err, res) => {
    if (err) {
        console.error(err);
    } else if (res.length > 0) {
        const tag = res[0].label; // __label__someTag
        const score = res[0].valuel // 1.3455345
    } else {
        console.log('No matches');
    }
});
```


## Nearest neighbour

There is a simple class for searching nearest neighbours:

```javascript
const path = require('path');
const { Query } = require('../main');

const model = path.resolve(__dirname, './skipgram.bin');

const query = new Query(model);

query.nn('word', 10, (err, res) => {
    if (err) {
        console.error(err);
    } else if (res.length > 0) {
        const tag = res[0].label;   // letter
        const score = res[0].valuel // 0.99992
    } else {
        console.log('No matches');
    }
});
```