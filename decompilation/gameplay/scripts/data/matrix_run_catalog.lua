-- Source tables at F25AE, F26C1, F26D6, F2602, F3D88/F3D90/F3D98,
-- C195D, and C19C6, transcribed as named host data. Indexes here are
-- zero-based only where they are part of the source objective catalog.
local function objective(class_id, value, systems, description)
    return {
        class_id = class_id,
        value = value,
        systems = systems,
        description = description,
    }
end

local system_order = {
    7, 12, 13, 24, 17, 19, 20, 21, 22, 23, 18, 0, 1,
    2, 3, 4, 5, 6, 8, 9, 10, 11, 14, 15, 16,
}

local systems = {
    [0] = "A local office",
    [1] = "A corporate branch office",
    [2] = "A small R&D firm",
    [3] = "A new local company",
    [4] = "A local retail business",
    [5] = "A local corporate office",
    [6] = "A local shipping warehouse",
    [7] = "Aztechnology",
    [8] = "A legal firm",
    [9] = "A local Stuffer Shack",
    [10] = "A local SimSense Company",
    [11] = "A local Coffin Hotel",
    [12] = "Club Penumbra",
    [13] = "Seattle Gen Hosp",
    [14] = "The Mayor's Office",
    [15] = "A SimSense Corporation",
    [16] = "UCAS Fed Govt",
    [17] = "Gates Undersound",
    [18] = "Ito's Passcode",
    [19] = "Hollywood",
    [20] = "Mitsuhama",
    [21] = "Renraku",
    [22] = "Fuchi",
    [23] = "Lone Star",
    [24] = "Ares",
}

local objectives = {
    objective(4, 20, { 2, 5, 7, 10, 11, 12, 14, 15, 16, 17, 19, 21, 22, 23, 24 },
        "is curious about how good their security is.||Go into their system, and retrieve a high security data file.||Bring it to me along with a briefing of their strengths and weaknesses, and you will be well paid."),
    objective(4, 21, { 2, 5, 7, 10, 11, 12, 14, 15, 16, 17, 19, 21, 22, 23, 24 },
        "has many informants working for them.||Go into their system and bring me a list of their security contacts.||You will be paid when I receive the file."),
    objective(0, 1, { 0, 6, 14, 12, 13, 16, 17, 19, 20, 21, 22, 23, 24 },
        "has an old file in their archives that I could use.||Bring me a copy of their archived files.||You will be paid upon delivery of that file."),
    objective(10, 52, { 7, 14, 20, 21, 24 },
        "is planning to kidnap someone.||Get into their confidential files and bring me a copy of their Strike Team orders.||You will be paid upon delivery of that file."),
    objective(10, 51, { 7, 14, 20, 21, 24 },
        "is funding it's own run which may affect a client of mine.||Bring me proof of this activity and you will be amply rewarded."),
    objective(10, 54, { 7, 14, 20, 21, 24 },
        "has been working on a confidential new utility for cyberdecks.||Bring me a copy of the code and you will be paid handsomely."),
    objective(10, 0, { 7, 14, 20, 21, 24 },
        "is blackmailing a client of mine.||Go into their system and delete that information from their confidential files.||Accomplish this and you will find him most generous."),
    objective(2, 12, { 0, 1, 3, 4, 6, 9, 14, 15, 13, 16, 17, 19, 20, 21, 22, 23, 24 },
        "has a new director of operations.||Bring me a copy of their personnel records from their management datastore.||At that time you will receive payment."),
    objective(7, 38, { 7, 15 },
        "is scheduling a new band to sponsor their upcoming advertising campaign.||Bring me the name of the band and their agent from their marketing datastore to receive payment."),
    objective(9, 48, { 0, 4, 6, 14, 12, 16, 17, 19, 20, 21, 22 },
        "is rumored to have been beefing up their security.||I'd rather not go looking for the why if it is just a rumor.||I need you to retrieve a finance file on security expenditures. Return here when you are finished for payment."),
    objective(-1, 0, { 7, 12, 13, 24, 17, 19, 20, 21, 22, 23, 18, 0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 14, 15 },
        "is holding an informant of mine. I have set up a team to acquire this person.||I need you to crash their CPU as a distraction.||Go do this now and return here for payment."),
    objective(12, 60, { 16, 19, 23 },
        "has been quite active lately.||Go into their system and bring me a list of current inmates so I can update my files.||You will be paid upon delivery of that information."),
    objective(3, 15, { 1, 2, 5, 6, 7, 14, 15, 16, 20, 21, 24 },
        "has its mages doing spell research.||Bring me a copy of their project logs and you will be paid handsomely."),
    objective(0, 3, { 0, 6, 14, 12, 13, 16, 17, 19, 20, 21, 22, 23, 24 },
        "has made me curious lately.||I would like for you to bring me a copy of their outdated history data.||You will be paid upon delivery of that file."),
    objective(8, 43, { 10, 15, 22 },
        "has been experimenting with revolutionary simsense input devices.||Bring me a copy of the data collected and I'll gladly reimburse you."),
    objective(8, 40, { 10, 15, 22 },
        "supposedly has a new simsense line-up this fall that may threaten certain corporations.||Bring me information on this and I'll gladly compensate you."),
    objective(13, 0, { 8, 23 },
        "is threatening me with a trial.||I need you to go into their system and delete my case file.||Return here to collect payment when this has been accomplished."),
    objective(9, -1, { 0, 4, 6, 14 },
        "My client embezzled funds from his employer and caught wind of a surprise audit at week's end.||He'll pay to have you go into their financial datastore to cover his acquisition by uploading this doctored file.||Return here when you are finished for payment."),
    objective(2, -1, { 0, 1, 3, 4, 6, 9, 14, 15 },
        "My client was wrongly fired from his job recently and wants to ruin the reputation of his former manager.||Upload this bogus personnel record into their management datastore and return here for payment."),
    objective(2, -1, { 0, 1, 3, 4, 6, 9, 14, 15 },
        "My client wants his management records updated.||Upload this file to record a substantial raise and log extra vacation time for him.||Return here to collect payment when this has been accomplished."),
}

local bands = {
    simple = { 0, 1, 2, 3, 4, 9, 11 },
    moderate = { 5, 6, 8, 10, 12, 13, 14 },
    expert = { 7, 15, 16, 17, 19, 20, 21, 22, 23, 24 },
}

return {
    objectives = objectives,
    systems = systems,
    system_order = system_order,
    bands = bands,
}
