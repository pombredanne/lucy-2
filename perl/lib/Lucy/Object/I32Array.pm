use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy PACKAGE = Lucy::Object::I32Array

SV*
new(either_sv, ...) 
    SV *either_sv;
CODE:
{
    SV *ints_sv = NULL;
    AV *ints_av = NULL;
    lucy_I32Array *self = NULL;

    XSBind_allot_params( &(ST(0)), 1, items, 
        "Lucy::Object::I32Array::new_PARAMS",
        &ints_sv, "ints", 4,
        NULL);
    if (XSBind_sv_defined(ints_sv) && SvROK(ints_sv)) {
        ints_av = (AV*)SvRV(ints_sv);
    }

    if (ints_av && SvTYPE(ints_av) == SVt_PVAV) {
        chy_i32_t size  = av_len(ints_av) + 1;
        chy_i32_t *ints = (chy_i32_t*)LUCY_MALLOCATE(size * sizeof(chy_i32_t));
        chy_i32_t i;

        for (i = 0; i < size; i++) {
            SV **const sv_ptr = av_fetch(ints_av, i, 0);
            ints[i] = (sv_ptr && XSBind_sv_defined(*sv_ptr)) 
                    ? SvIV(*sv_ptr) 
                    : 0;
        }
        self = (lucy_I32Array*)XSBind_new_blank_obj(either_sv);
        lucy_I32Arr_init(self, ints, size);
    }
    else {
        THROW(LUCY_ERR, "Required param 'ints' isn't an arrayref");
    }
    
    RETVAL = LUCY_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

SV*
to_arrayref(self)
    lucy_I32Array *self;
CODE:
{
    AV *out_av = newAV();
    chy_u32_t i;
    chy_u32_t size = Lucy_I32Arr_Get_Size(self);

    av_extend(out_av, size);
    for (i = 0; i < size; i++) {
        chy_i32_t result = Lucy_I32Arr_Get(self, i);
        SV* result_sv = result == -1 ? newSV(0) : newSViv(result);
        av_push(out_av, result_sv);
    }
    RETVAL = newRV_noinc((SV*)out_av);
}
OUTPUT: RETVAL
END_XS_CODE

Boilerplater::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Object::I32Array",
    xs_code      => $xs_code,
    bind_methods => [qw( Get Get_Size )],
);

__COPYRIGHT__

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

