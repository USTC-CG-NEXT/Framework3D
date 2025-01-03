import glints.scratch_grid

def test_scratch_field():
    field = glints.scratch_grid.ScratchField(10, 10)
    assert field.n == 10
    assert field.m == 10
    assert field.field.shape == (10, 10, 10, 2)


def test_render_scratch_field():
    renderer = None
    field = glints.scratch_grid.ScratchField(10, 10)
    glints.scratch_grid.render_scratch_field(renderer, field)
    

