
CREATE TABLE PstnRecord (
    begin_connection_time timestamp with time zone,
    duration integer,
    call_type_id integer,
    supplement_service_id text,
    in_abonent_type text,
    out_abonent_type text,
    switch_id text,
    inbound_trunk text,
    outbound_trunk text,
    term_cause text,
    in_info text,
    dialed_digits text,
    out_info text,
    forwarding_identifier text,
    message text,
    data_content_id integer
);
