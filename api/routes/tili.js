const express = require('express');
const router = express.Router();
const tili = require('../models/tili_model');

router.get('/', 
function(request, response) {
  tili.getAll (function (err, dbResult) {
    if (err) {
      response.json(err);
    } else {
      response.json(dbResult);
    }
  });
});

router.get('/:id?', function(request, response) {
    if (request.params.id) {
    tili.getByTilinumero (request.params.id, function (err, dbResult) {
        if (err) {
            response.json(err);
        } else {
            response.json(dbResult);
        }
    });
}
});

router.get('/saldo/:id?', function(request, response) {
  if (request.params.id) {
  tili.getSaldo (request.params.id, function (err, dbResult) {
      if (err) {
          response.json(err);
      } else {
          response.json(dbResult[0]);
      }
  });
}
});

router.post('/', 
function(request, response) {
  tili.add(request.body, function(err, dbResult) {
    if (err) {
      response.json(err);
    } else {
      response.json(request.body);
    }
  });
});


router.delete('/:id?', 
function(request, response) {
  tili.delete(request.params.id, function(err, dbResult) {
    if (err) {
      response.json(err);
    } else {
      response.json(dbResult);
    }
  });
});


router.put('/:id', 
function(request, response) {
  tili.update(request.params.id, request.body, function(err, dbResult) {
    if (err) {
      response.json(err);
    } else {
      response.json(dbResult);
    }
  });
});

module.exports = router;