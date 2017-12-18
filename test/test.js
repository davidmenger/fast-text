/*
 * @author David Menger
 */
'use strict';

const assert = require('assert');
const path = require('path');
const { Classifier, Query } = require('../main');

describe('<Classifier>', function () {

    it('should clasify the model', function (done) {
        const model = path.resolve(__dirname, './classification.bin');

        const c = new Classifier(model);

        c.predict('how it works', 1, (err, res) => {
            if (err) {
                done(err);
                return;
            }
            assert.equal(Array.isArray(res), true, 'res should be an array');
            assert.strictEqual(res.length, 1);
            res.forEach((v) => {
                assert.equal(typeof v, 'object');
                assert.equal(v.label, '__label__helloLabel');
                assert.equal(typeof v.value, 'number');
            });
            done();
        });
    });

    it('should not fall with bad words', function (done) {
        const model = path.resolve(__dirname, './classification.bin');

        const c = new Classifier(model);

        c.predict('wtf', 1, (err, res) => {
            if (err) {
                done(err);
                return;
            }
            assert.equal(Array.isArray(res), true, 'res should be an array');
            assert.strictEqual(res.length, 0);
            res.forEach((v) => {
                assert.equal(typeof v, 'object');
                assert.equal(v.label, '__label__helloLabel');
                assert.equal(typeof v.value, 'number');
            });
            done();
        });
    });

});

describe('<Query>', function () {

    this.timeout(10000);

    it('should return near neighbors', function (done) {

        const model = path.resolve(__dirname, './query.bin');

        const c = new Query(model);

        c.nn('wozniak', 2, (err, res) => {
            if (err) {
                done(err);
                return;
            }
            assert.equal(Array.isArray(res), true, 'res should be an array');
            assert.strictEqual(res.length, 2);
            res.forEach((v) => {
                assert.equal(typeof v, 'object');
                assert.equal(typeof v.label, 'string');
                assert.equal(typeof v.value, 'number');
            })
            done();;
        });
    });
});